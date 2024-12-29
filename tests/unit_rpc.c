#include "rpc.h"
#include <munit.h>

MunitResult unit_rpc_create_ping_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{1, 2, 3, 4, 5, 6, 7, 8}};
    int      request_id;
    char    *payload_str = create_ping_request(&sender_id, &request_id);

    void         *parse_data = kad_payload_parse(payload_str, strlen(payload_str) + 1);
    int           p_request_id;
    kad_request_t p_request;
    munit_assert_ptr_not_null(parse_data);
    munit_assert_true(kad_payload_is_request(parse_data));
    munit_assert_true(kad_payload_request_id(parse_data, &p_request_id));
    munit_assert_true(kad_payload_parse_request(parse_data, &p_request));
    munit_assert_int(p_request.type, ==, KAD_PING);
    munit_assert_int(kad_uint256_cmp(&sender_id, &p_request.d.ping.id), ==, 0);
    kad_request_fini(&p_request, parse_data);

    return MUNIT_OK;
}

MunitResult unit_rpc_create_store_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{1, 2, 3, 4, 5, 6, 7, 8}};
    int      request_id;
    char    *payload_str = create_store_request(&sender_id, "key", "value", &request_id);

    void         *parse_data = kad_payload_parse(payload_str, strlen(payload_str) + 1);
    int           p_request_id;
    kad_request_t p_request;
    munit_assert_ptr_not_null(parse_data);
    munit_assert_true(kad_payload_is_request(parse_data));
    munit_assert_true(kad_payload_request_id(parse_data, &p_request_id));
    munit_assert_true(kad_payload_parse_request(parse_data, &p_request));
    munit_assert_int(p_request.type, ==, KAD_STORE);
    munit_assert_int(kad_uint256_cmp(&sender_id, &p_request.d.store.id), ==, 0);
    munit_assert_string_equal(p_request.d.store.key, "key");
    munit_assert_string_equal(p_request.d.store.value, "value");
    kad_request_fini(&p_request, parse_data);

    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_node_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{1, 2, 3, 4, 5, 6, 7, 8}};
    kad_id_t target_id = {{2, 3, 4, 5, 6, 7, 8, 9}};
    int      request_id;
    char    *payload_str = create_find_node_request(&sender_id, &target_id, &request_id);

    void         *parse_data = kad_payload_parse(payload_str, strlen(payload_str) + 1);
    int           p_request_id;
    kad_request_t p_request;
    munit_assert_ptr_not_null(parse_data);
    munit_assert_true(kad_payload_is_request(parse_data));
    munit_assert_true(kad_payload_request_id(parse_data, &p_request_id));
    munit_assert_true(kad_payload_parse_request(parse_data, &p_request));
    munit_assert_int(p_request.type, ==, KAD_FIND_NODE);
    munit_assert_int(kad_uint256_cmp(&sender_id, &p_request.d.find_node.id), ==, 0);
    munit_assert_int(kad_uint256_cmp(&target_id, &p_request.d.find_node.target_id), ==, 0);
    kad_request_fini(&p_request, parse_data);

    return MUNIT_OK;
}

MunitResult unit_rpc_create_find_value_request(const MunitParameter params[], void *data)
{
    kad_id_t sender_id = {{1, 2, 3, 4, 5, 6, 7, 8}};
    int      request_id;
    char    *payload_str = create_find_value_request(&sender_id, "key", &request_id);

    void         *parse_data = kad_payload_parse(payload_str, strlen(payload_str) + 1);
    int           p_request_id;
    kad_request_t p_request;
    munit_assert_ptr_not_null(parse_data);
    munit_assert_true(kad_payload_is_request(parse_data));
    munit_assert_true(kad_payload_request_id(parse_data, &p_request_id));
    munit_assert_true(kad_payload_parse_request(parse_data, &p_request));
    munit_assert_int(p_request.type, ==, KAD_FIND_VALUE);
    munit_assert_int(kad_uint256_cmp(&sender_id, &p_request.d.find_value.id), ==, 0);
    munit_assert_string_equal(p_request.d.find_value.key, "key");
    kad_request_fini(&p_request, parse_data);

    return MUNIT_OK;
}

MunitTest unit_rpc_tests[] = {
    {"/create_ping_request", unit_rpc_create_ping_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_store_request", unit_rpc_create_store_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_find_node_request", unit_rpc_create_find_node_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/create_find_value_request", unit_rpc_create_find_value_request, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
