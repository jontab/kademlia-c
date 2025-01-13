#include "client.h"
#include "logging.h"
#include <munit.h>

typedef struct unit_insert_context_s unit_insert_context_t;

struct unit_insert_context_s
{
    kad_client_t client_1;
    kad_client_t client_2;
    int          port_1;
    int          port_2;
    char         host_1[BUFSIZ];
    char         host_2[BUFSIZ];
};

static void unit_client_insert_bootstrap_1(void *user);
static void unit_client_insert_bootstrap_2(void *user);
static void unit_client_insert_callback(void *user);
static void unit_client_insert_lookup_callback(const char *value, void *user);

MunitResult unit_client_insert(const MunitParameter params[], void *data)
{
    // Setup.
    unit_insert_context_t c = {0};
    c.port_1 = rand() % 1000 + 8000;
    c.port_2 = rand() % 1000 + 8000;
    strncpy(c.host_1, "0.0.0.0", sizeof(c.host_1) - 1);
    strncpy(c.host_2, "0.0.0.0", sizeof(c.host_2) - 1);

    kad_client_init(&c.client_1, uv_default_loop(), "0.0.0.0", c.port_1);
    kad_client_init(&c.client_2, uv_default_loop(), "0.0.0.0", c.port_2);

    INFO("client 1 is %U @ %d", c.client_1.id, c.port_1);
    INFO("client 2 is %U @ %d", c.client_2.id, c.port_2);

    kad_client_start(&c.client_1);
    kad_client_start(&c.client_2);

    INFO("Starting first bootstrap");
    const char *hosts[] = {c.host_2};
    kad_client_bootstrap(&c.client_1, hosts, &c.port_2, 1, unit_client_insert_bootstrap_1, &c);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    kad_client_fini(&c.client_1);
    kad_client_fini(&c.client_2);
    return MUNIT_OK;
}

MunitTest unit_client_tests[] = {
    {"/insert", unit_client_insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

//
// Statics
//

void unit_client_insert_bootstrap_1(void *user)
{
    INFO("Starting insert operation");
    unit_insert_context_t *c = (unit_insert_context_t *)(user);
    kad_client_insert(&c->client_1, "key", "value", unit_client_insert_callback, c);
}

void unit_client_insert_callback(void *user)
{
    INFO("Starting lookup operation");
    unit_insert_context_t *c = (unit_insert_context_t *)(user);
    kad_client_lookup(&c->client_1, "key", unit_client_insert_lookup_callback, c);
}

void unit_client_insert_lookup_callback(const char *value, void *user)
{
    munit_assert_ptr_not_null(value);
    munit_assert_string_equal(value, "value");
    uv_stop(uv_default_loop());
}
