#include "crawlers/valuecrawler.h"
#include "log.h"
#include "protocol.h"
#include "table.h"
#include <munit.h>

#define NCLIENTS   8
#define CRAWLERCAP 4

static void kad_valuecrawler_find_cb(const char *value, void *user);

MunitResult unit_valuecrawler_find(const MunitParameter params[], void *data)
{
    // Setup.
    int             ports[NCLIENTS];
    kad_storage_t   storages[NCLIENTS];
    kad_table_t     tables[NCLIENTS];
    kad_protocol_t *protocols[NCLIENTS];

    for (int i = 0; i < NCLIENTS; i++)
    {
        ports[i] = rand() % 1000 + 6000;
        kad_storage_init(&storages[i]);

        if (i == 0)
        {
            kad_id_t id = {0}; // We will be looking for this one.
            kad_table_init(&tables[i], &id, 8);
        }
        else
        {
            kad_id_t id = {{rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()}};
            kad_table_init(&tables[i], &id, 8);
        }

        kad_uv_protocol_t *protocol = kad_uv_protocol_new(uv_default_loop(), &tables[i], &storages[i]);
        protocols[i] = (kad_protocol_t *)(protocol);
        kad_uv_protocol_start(protocol, "0.0.0.0", ports[i]);

        kad_debug("table %d (%U) listening on port %d\n", i, &tables[i].id, ports[i]);
    }

    for (int i = 1; i < NCLIENTS; i++)
    {
        kad_contact_t prev = {.id = tables[i - 1].id, .host = "0.0.0.0", .port = ports[i - 1]};
        kad_table_add_contact(&tables[i], &prev);
    }

    kad_storage_put(&storages[0], "key", "value");

    // Execute.
    kad_contact_t seed = {
        .id = tables[NCLIENTS - 2].id,
        .host = "0.0.0.0",
        .port = ports[NCLIENTS - 2],
    };
    kad_valuecrawlerargs_t args = {
        .id = tables[NCLIENTS - 1].id,    // We're calling as the `NCLIENTS - 1` client.
        .proto = protocols[NCLIENTS - 1], // We're calling as the `NCLIENTS - 1` client.
        .key = "key",
        .capacity = CRAWLERCAP,
        .alpha = 2,
        .contacts = &seed,
        .contacts_size = 1,
    };
    kad_valuecrawler_t crawler;
    kad_valuecrawler_init(&crawler, &args);

    int i = 0;
    kad_valuecrawler_find(&crawler, kad_valuecrawler_find_cb, &i);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);

    // Cleanup.
    for (int i = 0; i < NCLIENTS; i++)
    {
        kad_uv_protocol_free((kad_uv_protocol_t *)(protocols[i]));
        kad_table_fini(&tables[i]);
    }

    return MUNIT_OK;
}

MunitTest unit_valuecrawler_tests[] = {
    {"/find", unit_valuecrawler_find, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

//
// Statics
//

void kad_valuecrawler_find_cb(const char *value, void *user)
{
    munit_assert_string_equal(value, "value");
    uv_stop(uv_default_loop());
}
