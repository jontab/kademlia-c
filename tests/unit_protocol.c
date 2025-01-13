#include "cJSON.h"
#include "logging.h"
#include "protocol.h"
#include "rpc.h"
#include <munit.h>

#define TEST_HOST1 "0.0.0.0"
#define TEST_HOST2 "0.0.0.0"

static void ping_cb(bool ok, void *result_p, void *data);
static void ping_cb_timeout(bool ok, void *result_p, void *data);
static void store_cb(bool ok, void *result_p, void *data);
static void find_node_cb(bool ok, void *result_p, void *data);
static void find_value_cb(bool ok, void *result_p, void *data);

MunitResult unit_protocol_send_ping(const MunitParameter params[], void *data)
{
    int test_port1 = rand() % 10000 + 8000;
    int test_port2 = rand() % 10000 + 8000;
    munit_assert_int(test_port1, !=, test_port2);

    // Setup.
    kad_table_t table_1;
    kad_table_t table_2;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_table_init(&table_2, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}, 9);
    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1, NULL);
    kad_uv_protocol_t *protocol_2 = kad_uv_protocol_new(uv_default_loop(), &table_2, NULL);
    kad_uv_protocol_start(protocol_1, TEST_HOST1, test_port1);
    kad_uv_protocol_start(protocol_2, TEST_HOST2, test_port2);

    // Execute.
    bool flag = false;
    protocol_1->base.ping(&(kad_ping_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .host = TEST_HOST2,
        .port = test_port2,
        .callback = ping_cb,
        .user = &flag,
    });
    protocol_2->base.ping(&(kad_ping_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_2.id,
        .host = TEST_HOST1,
        .port = test_port1,
        .callback = ping_cb,
        .user = &flag,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    munit_assert_true(kad_table_contains(&table_1, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}));
    munit_assert_true(kad_table_contains(&table_2, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}));

    // Cleanup.
    kad_uv_protocol_free(protocol_1);
    kad_uv_protocol_free(protocol_2);
    kad_table_fini(&table_1);
    kad_table_fini(&table_2);
    return MUNIT_OK;
}

MunitResult unit_protocol_send_ping_timeout(const MunitParameter params[], void *data)
{
    int test_port1 = rand() % 10000 + 8000;
    int test_port2 = rand() % 10000 + 8000;
    munit_assert_int(test_port1, !=, test_port2);

    // Setup.
    kad_table_t table_1;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1, NULL);
    kad_table_set_protocol(&table_1, (kad_protocol_t *)(protocol_1));
    kad_uv_protocol_start(protocol_1, TEST_HOST1, test_port1);

    // Execute.
    protocol_1->base.ping(&(kad_ping_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .host = TEST_HOST2,
        .port = test_port2,
        .callback = ping_cb_timeout,
        .user = NULL,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    // Cleanup.
    kad_uv_protocol_free(protocol_1);
    kad_table_fini(&table_1);
    return MUNIT_OK;
}

MunitResult unit_protocol_send_store(const MunitParameter params[], void *data)
{
    int test_port1 = rand() % 10000 + 8000;
    int test_port2 = rand() % 10000 + 8000;
    munit_assert_int(test_port1, !=, test_port2);

    // Setup.
    kad_table_t table_1;
    kad_table_t table_2;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_table_init(&table_2, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}, 9);
    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1, NULL);
    kad_uv_protocol_t *protocol_2 = kad_uv_protocol_new(uv_default_loop(), &table_2, NULL);
    kad_uv_protocol_start(protocol_1, TEST_HOST1, test_port1);
    kad_uv_protocol_start(protocol_2, TEST_HOST2, test_port2);

    // Execute.
    protocol_1->base.store(&(kad_store_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .key = "key",
        .value = "value",
        .host = TEST_HOST2,
        .port = test_port2,
        .callback = store_cb,
        .user = NULL,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    munit_assert_false(kad_table_contains(&table_1, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}));
    munit_assert_true(kad_table_contains(&table_2, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}));

    // Cleanup.
    kad_uv_protocol_free(protocol_1);
    kad_uv_protocol_free(protocol_2);
    kad_table_fini(&table_1);
    kad_table_fini(&table_2);
    return MUNIT_OK;
}

MunitResult unit_protocol_send_find_node(const MunitParameter params[], void *data)
{
    int test_port1 = rand() % 10000 + 8000;
    int test_port2 = rand() % 10000 + 8000;
    munit_assert_int(test_port1, !=, test_port2);

    // Setup.
    kad_table_t table_1;
    kad_table_t table_2;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_table_init(&table_2, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}, 8);
    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1, NULL);
    kad_uv_protocol_t *protocol_2 = kad_uv_protocol_new(uv_default_loop(), &table_2, NULL);
    kad_uv_protocol_start(protocol_1, TEST_HOST1, test_port1);
    kad_uv_protocol_start(protocol_2, TEST_HOST2, test_port2);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 2}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 3}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 4}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 5}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 0, 6}, .host = "localhost", .port = 8080},
    };

    for (int i = 0; i < sizeof(conts) / sizeof(conts[0]); i++)
    {
        kad_table_add_contact(&table_2, &conts[i]);
    }

    // Execute.
    protocol_1->base.find_node(&(kad_find_node_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .target_id = &(kad_id_t){{0, 0, 0, 0, 0, 0, 0, 0}},
        .host = TEST_HOST2,
        .port = test_port2,
        .callback = find_node_cb,
        .user = NULL,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    munit_assert_false(kad_table_contains(&table_1, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}));
    munit_assert_true(kad_table_contains(&table_2, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}));

    // Cleanup.
    kad_uv_protocol_free(protocol_1);
    kad_uv_protocol_free(protocol_2);
    kad_table_fini(&table_1);
    kad_table_fini(&table_2);
    return MUNIT_OK;
}

MunitResult unit_protocol_send_find_value(const MunitParameter params[], void *data)
{
    int test_port1 = rand() % 10000 + 8000;
    int test_port2 = rand() % 10000 + 8000;
    munit_assert_int(test_port1, !=, test_port2);

    // Setup.
    kad_storage_t storage_1;
    kad_storage_t storage_2;
    kad_storage_init(&storage_1);
    kad_storage_init(&storage_2);

    kad_table_t table_1;
    kad_table_t table_2;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_table_init(&table_2, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}, 8);

    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1, &storage_1);
    kad_uv_protocol_t *protocol_2 = kad_uv_protocol_new(uv_default_loop(), &table_2, &storage_2);
    kad_uv_protocol_start(protocol_1, TEST_HOST1, test_port1);
    kad_uv_protocol_start(protocol_2, TEST_HOST2, test_port2);

    // Execute.
    kad_storage_put(&storage_2, "key", "value");

    protocol_1->base.find_value(&(kad_find_value_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .key = "key",
        .host = TEST_HOST2,
        .port = test_port2,
        .callback = find_value_cb,
        .user = NULL,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    munit_assert_false(kad_table_contains(&table_1, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}));
    munit_assert_true(kad_table_contains(&table_2, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}));

    // Cleanup.
    kad_uv_protocol_free(protocol_1);
    kad_uv_protocol_free(protocol_2);

    kad_table_fini(&table_1);
    kad_table_fini(&table_2);

    kad_storage_fini(&storage_1);
    kad_storage_fini(&storage_2);
    return MUNIT_OK;
}

MunitTest unit_protocol_tests[] = {
    {"/send_ping", unit_protocol_send_ping, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/send_ping_timeout", unit_protocol_send_ping_timeout, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/send_store", unit_protocol_send_store, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/send_find_node", unit_protocol_send_find_node, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/send_find_value", unit_protocol_send_find_value, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

//
// Static
//

void ping_cb(bool ok, void *result_p, void *data)
{
    munit_assert_true(ok);

    bool         *flag = (bool *)(data);
    kad_result_t *result = (kad_result_t *)(result_p);
    munit_assert_int(result->type, ==, KAD_PING);

    kad_id_t server_id1 = {{2, 3, 4, 5, 6, 7, 8, 9}};
    kad_id_t server_id2 = {{1, 2, 3, 4, 5, 6, 7, 8}};
    bool     eq_server_id1 = kad_uint256_cmp(&result->d.ping.id, &server_id1) == 0;
    bool     eq_server_id2 = kad_uint256_cmp(&result->d.ping.id, &server_id2) == 0;
    munit_assert_true(eq_server_id1 || eq_server_id2);

    if (*flag == false) // First.
    {
        *flag = true;
    }
    else // Second.
    {
        uv_stop(uv_default_loop());
    }
}

void ping_cb_timeout(bool ok, void *result_p, void *data)
{
    munit_assert_false(ok);
    munit_assert_ptr_null(result_p);
    munit_assert_ptr_null(data);
    uv_stop(uv_default_loop());
}

void store_cb(bool ok, void *result_p, void *data)
{
    munit_assert_true(ok);
    kad_result_t *result = (kad_result_t *)(result_p);
    munit_assert_int(result->type, ==, KAD_STORE);
    uv_stop(uv_default_loop());
}

void find_node_cb(bool ok, void *result_p, void *data)
{
    munit_assert_true(ok);
    kad_result_t *result = (kad_result_t *)(result_p);
    munit_assert_int(result->type, ==, KAD_FIND_NODE);
    munit_assert_int(result->d.find_node.size, ==, 7);
    uv_stop(uv_default_loop());
}

void find_value_cb(bool ok, void *result_p, void *data)
{
    munit_assert_true(ok);
    kad_result_t *result = (kad_result_t *)(result_p);
    munit_assert_int(result->type, ==, KAD_FIND_VALUE);
    munit_assert_ptr_not_null(result->d.find_value.value);
    munit_assert_string_equal(result->d.find_value.value, "value");
    munit_assert_int(result->d.find_value.size, ==, 0);
    uv_stop(uv_default_loop());
}
