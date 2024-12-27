#include "rpc.h"
#include <munit.h>

MunitResult unit_rpc_create_ping_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{2, 1, 2, 1, 2, 1, 2, 1}};
    cJSON   *monitor = create_ping_request(&sender_id);

    char *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    munit_assert_true(cJSON_IsNull(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "ping");
    munit_assert_string_equal(p_sender_id, "0x0000000200000001000000020000000100000002000000010000000200000001");

    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_node_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{2, 1, 2, 1, 2, 1, 2, 1}};
    kad_id_t target_id = {{4, 3, 4, 3, 4, 3, 4, 3}};
    cJSON   *monitor = create_find_node_request(&sender_id, &target_id);

    char *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    munit_assert_true(cJSON_IsNull(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_target_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "find_node");
    munit_assert_string_equal(p_sender_id, "0x0000000200000001000000020000000100000002000000010000000200000001");
    munit_assert_string_equal(p_target_id, "0x0000000400000003000000040000000300000004000000030000000400000003");

    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_store_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{2, 1, 2, 1, 2, 1, 2, 1}};
    cJSON   *monitor = create_store_request(&sender_id, "key", "value");

    char *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    munit_assert_true(cJSON_IsNull(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_key = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));
    char  *p_value = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 2));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "store");
    munit_assert_string_equal(p_sender_id, "0x0000000200000001000000020000000100000002000000010000000200000001");
    munit_assert_string_equal(p_key, "key");
    munit_assert_string_equal(p_value, "value");

    cJSON_Delete(monitor);
    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_value_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{2, 1, 2, 1, 2, 1, 2, 1}};
    cJSON   *monitor = create_find_value_request(&sender_id, "key");

    char *p_version = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "jsonrpc"));
    char *p_method = cJSON_GetStringValue(cJSON_GetObjectItem(monitor, "method"));
    munit_assert_true(cJSON_IsNull(cJSON_GetObjectItem(monitor, "id")));
    cJSON *p_params = cJSON_GetObjectItem(monitor, "params");
    char  *p_sender_id = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 0));
    char  *p_key = cJSON_GetStringValue(cJSON_GetArrayItem(p_params, 1));

    munit_assert_string_equal(p_version, "2.0");
    munit_assert_string_equal(p_method, "find_value");
    munit_assert_string_equal(p_sender_id, "0x0000000200000001000000020000000100000002000000010000000200000001");
    munit_assert_string_equal(p_key, "key");

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
