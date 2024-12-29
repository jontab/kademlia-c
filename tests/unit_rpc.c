#include "cJSON.h"
#include "rpc.h"
#include <munit.h>

MunitResult unit_rpc_create_ping_request(const MunitParameter params[], void *data)
{
    int    request_id;
    char  *payload = create_ping_request(&(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, &request_id);
    cJSON *monitor = cJSON_Parse(payload);

    char  *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char  *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    int    p_request_id = (int)(cJSON_GetNumberValue(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "ping");
    munit_assert_int(p_request_id, ==, request_id);
    munit_assert_string_equal(p_sender_id, "0x0000000100000002000000030000000400000005000000060000000700000008");

    free(payload);
    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_node_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{1, 2, 3, 4, 5, 6, 7, 8}};
    kad_id_t target_id = {{2, 3, 4, 5, 6, 7, 8, 9}};
    int      request_id;
    char    *payload = create_find_node_request(&sender_id, &target_id, &request_id);
    cJSON   *monitor = cJSON_Parse(payload);

    char  *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char  *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    int    p_request_id = (int)(cJSON_GetNumberValue(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_target_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "find_node");
    munit_assert_int(p_request_id, ==, request_id);
    munit_assert_string_equal(p_sender_id, "0x0000000100000002000000030000000400000005000000060000000700000008");
    munit_assert_string_equal(p_target_id, "0x0000000200000003000000040000000500000006000000070000000800000009");

    free(payload);
    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_store_request(const MunitParameter params[], void *data)
{
    int    request_id;
    char  *payload = create_store_request(&(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, "key", "value", &request_id);
    cJSON *monitor = cJSON_Parse(payload);

    char  *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char  *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    int    p_request_id = (int)(cJSON_GetNumberValue(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_key = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));
    char  *p_value = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 2));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "store");
    munit_assert_int(p_request_id, ==, request_id);
    munit_assert_string_equal(p_sender_id, "0x0000000100000002000000030000000400000005000000060000000700000008");
    munit_assert_string_equal(p_key, "key");
    munit_assert_string_equal(p_value, "value");

    free(payload);
    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_value_request(const MunitParameter params[], void *data)
{
    int    request_id;
    char  *payload = create_find_value_request(&(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, "key", &request_id);
    cJSON *monitor = cJSON_Parse(payload);

    char  *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char  *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    int    p_request_id = (int)(cJSON_GetNumberValue(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_key = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "find_value");
    munit_assert_int(p_request_id, ==, request_id);
    munit_assert_string_equal(p_sender_id, "0x0000000100000002000000030000000400000005000000060000000700000008");
    munit_assert_string_equal(p_key, "key");

    free(payload);
    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitTest unit_rpc_tests[] = {
    {"/create_ping_request", unit_rpc_create_ping_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_find_node_request", unit_rpc_create_find_node_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_store_request", unit_rpc_create_store_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_find_value_request", unit_rpc_create_find_value_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
