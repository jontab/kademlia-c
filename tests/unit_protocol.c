#include "cJSON.h"
#include "log.h"
#include "protocol.h"
#include "rpc.h"
#include <munit.h>

#define TEST_HOST1 "0.0.0.0"
#define TEST_PORT1 8090
#define TEST_HOST2 "0.0.0.0"
#define TEST_PORT2 8091

static void *unit_protocol_setup(const MunitParameter params[], void *data);
static void  unit_protocol_teardown(void *data);
static void  ping_cb(bool ok, void *data);

MunitResult unit_protocol_send_ping(const MunitParameter params[], void *data)
{
    kad_table_t table_1;
    kad_table_t table_2;
    kad_table_init(&table_1, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);
    kad_table_init(&table_2, &(kad_id_t){{2, 3, 4, 5, 6, 7, 8, 9}}, 9);

    kad_uv_protocol_t *protocol_1 = kad_uv_protocol_new(uv_default_loop(), &table_1);
    kad_uv_protocol_t *protocol_2 = kad_uv_protocol_new(uv_default_loop(), &table_2);
    kad_table_set_protocol(&table_1, (kad_protocol_t *)(protocol_1));
    kad_table_set_protocol(&table_2, (kad_protocol_t *)(protocol_2));

    kad_uv_protocol_start(protocol_1, TEST_HOST1, TEST_PORT1);
    kad_uv_protocol_start(protocol_2, TEST_HOST2, TEST_PORT2);

    bool flag = false;
    protocol_1->base.ping(&(kad_ping_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_1.id,
        .host = TEST_HOST2,
        .port = TEST_PORT2,
        .callback = ping_cb,
        .user = &flag,
    });
    protocol_2->base.ping(&(kad_ping_args_t){
        .self = (kad_protocol_t *)(protocol_1),
        .id = &table_2.id,
        .host = TEST_HOST1,
        .port = TEST_PORT1,
        .callback = ping_cb,
        .user = &flag,
    });

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    kad_uv_protocol_free(protocol_1);
    kad_uv_protocol_free(protocol_2);

    kad_table_fini(&table_1);
    kad_table_fini(&table_2);
    return MUNIT_OK;
}

MunitTest unit_protocol_tests[] = {
    {"/send_ping", unit_protocol_send_ping, unit_protocol_setup, unit_protocol_teardown, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

//
// Static
//

void *unit_protocol_setup(const MunitParameter params[], void *data)
{
    return NULL;
}

void unit_protocol_teardown(void *data)
{
}

void ping_cb(bool ok, void *data)
{
    munit_assert_true(ok);

    kad_debug("ping_cb");

    bool *flag = (bool *)(data);
    if (*flag == false) // First.
    {
        *flag = true;
    }
    else // Second.
    {
        uv_stop(uv_default_loop());
    }
}
