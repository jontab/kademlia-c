#include "rpc.h"
#include <assert.h>
#include <stdio.h>

#define ID_SERIALIZE_LEN (sizeof(kad_uint256_t) * 2 + 3) // "0x" + '\0'.

static void kad_id_serialize(kad_id_t *id, char *buffer);

//
// Public
//

cJSON *create_ping_request(kad_id_t *sender_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "ping");
    assert(ret && "Out of memory");
    ret = cJSON_AddNullToObject(monitor, "id");
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");

    return monitor;
}

cJSON *create_find_node_request(kad_id_t *sender_id, kad_id_t *target_id)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    char serialized_target_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);
    kad_id_serialize(target_id, serialized_target_id);

    cJSON     *ret;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "find_node");
    assert(ret && "Out of memory");
    ret = cJSON_AddNullToObject(monitor, "id");
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(serialized_target_id));
    assert(retbool && "Out of memory");

    return monitor;
}

cJSON *create_store_request(kad_id_t *sender_id, const char *key, const char *value)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "store");
    assert(ret && "Out of memory");
    ret = cJSON_AddNullToObject(monitor, "id");
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(key));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 2, cJSON_CreateString(value));
    assert(retbool && "Out of memory");

    return monitor;
}

cJSON *create_find_value_request(kad_id_t *sender_id, const char *key)
{
    cJSON *monitor = cJSON_CreateObject();
    assert(monitor && "Out of memory");

    char serialized_sender_id[ID_SERIALIZE_LEN];
    kad_id_serialize(sender_id, serialized_sender_id);

    cJSON     *ret;
    cJSON_bool retbool;

    ret = cJSON_AddStringToObject(monitor, "jsonrpc", "2.0");
    assert(ret && "Out of memory");
    ret = cJSON_AddStringToObject(monitor, "method", "find_value");
    assert(ret && "Out of memory");
    ret = cJSON_AddNullToObject(monitor, "id");
    assert(ret && "Out of memory");
    ret = cJSON_AddArrayToObject(monitor, "params");
    assert(ret && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 0, cJSON_CreateString(serialized_sender_id));
    assert(retbool && "Out of memory");
    retbool = cJSON_InsertItemInArray(ret, 1, cJSON_CreateString(key));
    assert(retbool && "Out of memory");

    return monitor;
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
