#include "rpc.h"
#include "alloc.h"
#include "cJSON.h"
#include "log.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_SERIALIZE_LEN (sizeof(kad_uint256_t) * 2 + 3) // "0x" + '\0'.

typedef struct parse_context_s parse_context_t;

struct parse_context_s
{
    cJSON *monitor;
    cJSON *p_request_id;
    cJSON *p_method;
    cJSON *p_params;
    cJSON *p_result;
};

static void kad_id_serialize(kad_id_t *id, char *buffer);
static bool kad_id_deserialize(const char *buf, kad_id_t *out);
static bool payload_parse_request(cJSON *monitor, kad_request_t *out);
static bool payload_parse_result(cJSON *monitor, kad_result_t *out);

//
// Public
//

char *create_ping_request(kad_id_t *sender_id, int *out_request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;
    int        request_id = rand();

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "ping");
    assert(ret && "Out of memory");
    ret = cJSON_AddNumberToObject(monitor, "id", (double)(request_id));
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");

    char *out = cJSON_PrintUnformatted(monitor);
    if (out_request_id)
    {
        *out_request_id = request_id;
    }

    cJSON_Delete(monitor);
    return out;
}

char *create_store_request(kad_id_t *sender_id, const char *key, const char *value, int *out_request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;
    int        request_id = rand();

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "store");
    assert(ret && "Out of memory");
    ret = cJSON_AddNumberToObject(monitor, "id", (double)(request_id));
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(key));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 2, cJSON_CreateString(value));
    assert(retbool && "Out of memory");

    char *out = cJSON_PrintUnformatted(monitor);
    if (out_request_id)
    {
        *out_request_id = request_id;
    }

    cJSON_Delete(monitor);
    return out;
}

char *create_find_node_request(kad_id_t *sender_id, kad_id_t *target_id, int *out_request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    char serialized_target_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);
    kad_id_serialize(target_id, serialized_target_id);

    cJSON     *ret;
    cJSON_bool retbool;
    int        request_id = rand();

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "find_node");
    assert(ret && "Out of memory");
    ret = cJSON_AddNumberToObject(monitor, "id", (double)(request_id));
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(serialized_target_id));
    assert(retbool && "Out of memory");

    char *out = cJSON_PrintUnformatted(monitor);
    if (out_request_id)
    {
        *out_request_id = request_id;
    }

    cJSON_Delete(monitor);
    return out;
}

char *create_find_value_request(kad_id_t *sender_id, const char *key, int *out_request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;
    int        request_id = rand();

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "find_value");
    assert(ret && "Out of memory");
    ret = cJSON_AddNumberToObject(monitor, "id", (double)(request_id));
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(key));
    assert(retbool && "Out of memory");

    char *out = cJSON_PrintUnformatted(monitor);
    if (out_request_id)
    {
        *out_request_id = request_id;
    }

    cJSON_Delete(monitor);
    return out;
}

char *create_ping_response(kad_id_t *server_id, int request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_server_id[ID_SERIALIZE_LEN];
    kad_id_serialize(server_id, serialized_server_id);

    cJSON     *ret;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "result", serialized_server_id);
    assert(ret && "Out of memory");
    ret = cJSON_AddNumberToObject(monitor, "id", request_id);
    assert(ret && "Out of memory");

    char *out = cJSON_PrintUnformatted(monitor);
    cJSON_Delete(monitor);
    return out;
}

char *create_store_response(int request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    kad_check(monitor, "cJSON_CreateObject failed");

    cJSON *ret;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    kad_check(ret, "cJSON_AddStringToObject failed");
    ret = cJSON_AddNullToObject(monitor, "result");
    kad_check(ret, "cJSON_AddNullToObject failed");
    ret = cJSON_AddNumberToObject(monitor, "id", request_id);
    kad_check(ret, "cJSON_AddNumberToObject failed");

    char *out = cJSON_PrintUnformatted(monitor);
    kad_check(out, "cJSON_PrintUnformatted failed");
    cJSON_Delete(monitor);
    return out;
}

char *create_find_node_response(kad_contact_t *contacts, int size, int request_id)
{
    cJSON *monitor = cJSON_CreateObject();
    kad_check(monitor, "cJSON_CreateObject failed");

    cJSON     *ret;
    cJSON     *resarray;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    kad_check(ret, "cJSON_AddStringToObject failed");
    ret = cJSON_AddNumberToObject(monitor, "id", request_id);
    kad_check(ret, "cJSON_AddNumberToObject failed");
    resarray = cJSON_AddArrayToObject(monitor, "result");
    kad_check(ret, "cJSON_AddArrayToObject failed");

    for (int i = 0; i < size; i++)
    {
        kad_contact_t *contact = &contacts[i];
        cJSON         *contarray = cJSON_CreateArray();
        kad_check(contarray, "cJSON_CreateArray failed");

        // [contact->id, contact->host, contact->port].
        char serialized_id[ID_SERIALIZE_LEN];
        kad_id_serialize(&contact->id, serialized_id);

        retbool = cJSON_AddItemToArray(contarray, cJSON_CreateString(serialized_id));
        assert(retbool && "cJSON_AddItemToArray failed");
        retbool = cJSON_AddItemToArray(contarray, cJSON_CreateString(contact->host));
        assert(retbool && "cJSON_AddItemToArray failed");
        retbool = cJSON_AddItemToArray(contarray, cJSON_CreateNumber((double)(contact->port)));
        assert(retbool && "cJSON_AddItemToArray failed");

        retbool = cJSON_AddItemToArray(resarray, contarray);
        assert(retbool && "cJSON_AddItemToArray failed");
    }

    char *out = cJSON_PrintUnformatted(monitor);
    kad_check(out, "cJSON_PrintUnformatted failed");
    cJSON_Delete(monitor);
    return out;
}

char *create_find_value_response(const char *value, kad_contact_t *contacts, int size, int request_id)
{
    return NULL;
}

//
// Parsing
//

void *kad_payload_parse(const char *buf, int size)
{
    parse_context_t *context = kad_alloc(1, sizeof(parse_context_t));
    context->monitor = cJSON_ParseWithLength(buf, size);
    if (context->monitor)
    {
        context->p_request_id = cJSON_GetObjectItem(context->monitor, "id");
        context->p_method = cJSON_GetObjectItem(context->monitor, "method");
        context->p_params = cJSON_GetObjectItem(context->monitor, "params");
        context->p_result = cJSON_GetObjectItem(context->monitor, "result");
        return (void *)(context);
    }
    else
    {
        free(context);
        return NULL;
    }
}

bool kad_payload_is_request(void *data)
{
    parse_context_t *context = (parse_context_t *)(data);
    return context->p_method != NULL;
}

bool kad_payload_request_id(void *data, int *request_id)
{
    parse_context_t *context = (parse_context_t *)(data);
    if (!context->p_request_id || !cJSON_IsNumber(context->p_request_id))
    {
        kad_warn("kad_payload_request_id: id is not a number\n");
        return false;
    }

    *request_id = (int)(cJSON_GetNumberValue(context->p_request_id));
    return true;
}

bool kad_payload_parse_request(void *data, kad_request_t *out)
{
    parse_context_t *context = (parse_context_t *)(data);
    cJSON           *p_method = context->p_method;
    cJSON           *p_params = context->p_params;
    if (!p_method || !cJSON_IsString(p_method))
    {
        kad_warn("payload_parse_request: method is not a string\n");
        return false;
    }

    if (!p_params || !cJSON_IsArray(p_params))
    {
        kad_warn("payload_parse_request: params is not an array\n");
        return false;
    }

    char *method = cJSON_GetStringValue(p_method);
    if (strcmp(method, "ping") == 0)
    {
        cJSON *p_sender_id = cJSON_GetArrayItem(p_params, 0);
        if (!p_sender_id || !cJSON_IsString(p_sender_id))
        {
            kad_warn("payload_parse_request: sender_id is not a string\n");
            return false;
        }

        char *s_sender_id = cJSON_GetStringValue(p_sender_id);
        if (!kad_id_deserialize(s_sender_id, &out->d.ping.id))
        {
            kad_warn("payload_parse_request: failed to parse sender_id\n");
            return false;
        }

        out->type = KAD_PING;
        return true;
    }

    if (strcmp(method, "store") == 0)
    {
        cJSON *p_sender_id = cJSON_GetArrayItem(p_params, 0);
        cJSON *p_key = cJSON_GetArrayItem(p_params, 1);
        cJSON *p_value = cJSON_GetArrayItem(p_params, 2);
        if (!p_sender_id || !cJSON_IsString(p_sender_id))
        {
            kad_warn("payload_parse_request: sender_id is not a string\n");
            return false;
        }

        if (!p_key || !cJSON_IsString(p_key))
        {
            kad_warn("payload_parse_request: key is not a string\n");
            return false;
        }

        if (!p_value || !cJSON_IsString(p_value))
        {
            kad_warn("payload_parse_request: value is not a string\n");
            return false;
        }

        char *s_sender_id = cJSON_GetStringValue(p_sender_id);
        if (!kad_id_deserialize(s_sender_id, &out->d.store.id))
        {
            kad_warn("payload_parse_request: failed to parse sender_id\n");
            return false;
        }

        out->type = KAD_STORE;
        out->d.store.key = strdup(cJSON_GetStringValue(p_key));
        out->d.store.value = strdup(cJSON_GetStringValue(p_value));
        kad_check(out->d.store.key, "cJSON_Print failed");
        kad_check(out->d.store.value, "cJSON_Print failed");
        return true;
    }

    if (strcmp(method, "find_node") == 0)
    {
        cJSON *p_sender_id = cJSON_GetArrayItem(p_params, 0);
        cJSON *p_target_id = cJSON_GetArrayItem(p_params, 1);
        if (!p_sender_id || !cJSON_IsString(p_sender_id))
        {
            kad_warn("payload_parse_request: sender_id is not a string\n");
            return false;
        }

        if (!p_target_id || !cJSON_IsString(p_target_id))
        {
            kad_warn("payload_parse_request: target_id is not a string\n");
            return false;
        }

        char *s_sender_id = cJSON_GetStringValue(p_sender_id);
        if (!kad_id_deserialize(s_sender_id, &out->d.find_node.id))
        {
            kad_warn("payload_parse_request: failed to parse sender_id\n");
            return false;
        }

        char *s_target_id = cJSON_GetStringValue(p_target_id);
        if (!kad_id_deserialize(s_target_id, &out->d.find_node.target_id))
        {
            kad_warn("payload_parse_request: failed to parse target_id\n");
            return false;
        }

        out->type = KAD_FIND_NODE;
        return true;
    }

    if (strcmp(method, "find_value") == 0)
    {
        cJSON *p_sender_id = cJSON_GetArrayItem(p_params, 0);
        cJSON *p_key = cJSON_GetArrayItem(p_params, 1);
        if (!p_sender_id || !cJSON_IsString(p_sender_id))
        {
            kad_warn("payload_parse_request: sender_id is not a string\n");
            return false;
        }

        if (!p_key || !cJSON_IsString(p_key))
        {
            kad_warn("payload_parse_request: key is not a string\n");
            return false;
        }

        char *s_sender_id = cJSON_GetStringValue(p_sender_id);
        if (!kad_id_deserialize(s_sender_id, &out->d.find_node.id))
        {
            kad_warn("payload_parse_request: failed to parse sender_id\n");
            return false;
        }

        out->d.find_value.key = strdup(cJSON_GetStringValue(p_key));
        kad_check(out->d.find_value.key, "cJSON_Print failed");
        out->type = KAD_FIND_VALUE;
        return true;
    }

    return false;
}

bool kad_payload_parse_result(void *data, int type, kad_result_t *out)
{
    parse_context_t *context = (parse_context_t *)(data);
    if (!context->p_result)
    {
        kad_warn("kad_payload_parse_result: missing result\n");
        return false;
    }

    switch (type)
    {
    case KAD_PING: {
        if (!cJSON_IsString(context->p_result))
        {
            kad_warn("kad_payload_parse_result: ping: result is not a string\n");
            return false;
        }

        char *s_server_id = cJSON_GetStringValue(context->p_result);
        if (!kad_id_deserialize(s_server_id, &out->d.ping.id))
        {
            kad_warn("kad_payload_parse_result: ping: failed to parse server_id\n");
            return false;
        }

        break;
    }
    case KAD_STORE:
        break;
    case KAD_FIND_NODE:
        break;
    case KAD_FIND_VALUE:
        break;
    default:
        kad_warn("kad_payload_parse_result: unhandled type: %d\n", type);
        break;
    }

    out->type = type;
    return true;
}

void kad_request_fini(kad_request_t *s, void *data)
{
    if (s)
    {
        switch (s->type)
        {
        case KAD_PING:
            break;
        case KAD_STORE:
            free(s->d.store.key);
            free(s->d.store.value);
            break;
        case KAD_FIND_NODE:
            break;
        case KAD_FIND_VALUE:
            free(s->d.find_value.key);
            break;
        default:
            kad_warn("kad_request_fini: unhandled type: %d\n", s->type);
            break;
        }
    }

    if (data)
    {
        parse_context_t *context = (parse_context_t *)(data);
        cJSON_Delete(context->monitor);
        free(context);
    }
}

void kad_result_fini(kad_result_t *s, void *data)
{
    if (s)
    {
        // TODO: Implement.
    }

    if (data)
    {
        parse_context_t *context = (parse_context_t *)(data);
        cJSON_Delete(context->monitor);
        free(context);
    }
}

//
// Static
//

void kad_id_serialize(kad_id_t *id, char *buffer)
{
    *(buffer++) = '0';
    *(buffer++) = 'x';
    for (int i = 0; i < sizeof(id->d) / sizeof(id->d[0]); i++)
    {
        sprintf(buffer, "%08X", id->d[i]);
        buffer += sizeof(id->d[0]) * 2;
    }

    *buffer = '\0';
}

bool kad_id_deserialize(const char *buf, kad_id_t *out)
{
    if (buf == NULL || (*buf == '\0') || (*(++buf) == '\0') || (*(++buf) == '\0'))
    {
        return false;
    }

    for (int i = 0; i < sizeof(out->d) / sizeof(out->d[0]); i++, buf += 8)
    {
        if (strlen(buf) < 8)
        {
            return false;
        }

        char int32_str[9] = {0};
        int32_str[0] = buf[0];
        int32_str[1] = buf[1];
        int32_str[2] = buf[2];
        int32_str[3] = buf[3];
        int32_str[4] = buf[4];
        int32_str[5] = buf[5];
        int32_str[6] = buf[6];
        int32_str[7] = buf[7];
        out->d[i] = strtol(int32_str, NULL, 16);
    }

    return true;
}
