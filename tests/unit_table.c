#include "logging.h"
#include "table.h"
#include "uint256.h"
#include <munit.h>

MunitResult unit_table_contains(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){0}, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 1, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 2, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 3, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 4, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 5, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 6, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {7, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        munit_assert_false(kad_table_contains(&t, &conts[i].id));
        kad_table_add_contact(&t, &conts[i]);
        munit_assert_true(kad_table_contains(&t, &conts[i].id));
    }

    // Cleanup.
    kad_table_fini(&t);
    return MUNIT_OK;
}

MunitResult unit_table_add_contact(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()}, 8);

    // Execute.
    for (int i = 0; i < 10000; i++)
    {
        kad_uint256_t id = {rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()};
        kad_contact_t c = {id, "localhost", 8080};
        kad_table_add_contact(&t, &c);
    }

    munit_assert_int(t.nbuckets, >, 0);

    // Cleanup.
    kad_table_fini(&t);
    return MUNIT_OK;
}

MunitResult unit_table_remove_contact(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){0}, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 1, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 2, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 3, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 4, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 5, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 6, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {7, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        kad_table_add_contact(&t, &conts[i]);
        munit_assert_true(kad_table_contains(&t, &conts[i].id));
        kad_table_remove_contact(&t, &conts[i].id);
        munit_assert_false(kad_table_contains(&t, &conts[i].id));
    }

    // Cleanup.
    kad_table_fini(&t);
    return MUNIT_OK;
}

void unit_table_find_closest_check(kad_contactlist_t *list)
{
    kad_contact_t *contacts = list->data;
    int            contacts_size = list->size;

    munit_assert_int(contacts_size, ==, 4);
    kad_id_t id_0 = contacts[0].id;
    kad_id_t id_1 = contacts[1].id;
    kad_id_t id_2 = contacts[2].id;
    kad_id_t id_3 = contacts[3].id;

    // 0.
    kad_id_t want_0 = {{0x80000000, 0, 0, 0, 0, 0, 2, 0}};
    munit_assert_int(kad_uint256_cmp(&id_0, &want_0), ==, 0);

    // 1.
    kad_id_t want_1 = {{0x80000000, 0, 0, 0, 0, 3, 0, 0}};
    munit_assert_int(kad_uint256_cmp(&id_1, &want_1), ==, 0);

    // 2.
    kad_id_t want_2 = {{0x80000000, 0, 0, 0, 4, 0, 0, 0}};
    munit_assert_int(kad_uint256_cmp(&id_2, &want_2), ==, 0);

    // 3.
    kad_id_t want_3 = {{0x00000000, 0, 0, 0, 0, 0, 0, 1}};
    munit_assert_int(kad_uint256_cmp(&id_3, &want_3), ==, 0);
}

MunitResult unit_table_find_closest(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){0}, 4);

    kad_contact_t conts[] = {
        {.id = {0x00000000, 0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = 8080},
        {.id = {0x00000000, 0, 0, 0, 0, 0, 2, 0}, .host = "localhost", .port = 8080},
        {.id = {0x00000000, 0, 0, 0, 0, 3, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0x00000000, 0, 0, 0, 4, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0x80000000, 0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = 8080},
        {.id = {0x80000000, 0, 0, 0, 0, 0, 2, 0}, .host = "localhost", .port = 8080},
        {.id = {0x80000000, 0, 0, 0, 0, 3, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0x80000000, 0, 0, 0, 4, 0, 0, 0}, .host = "localhost", .port = 8080},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_table_add_contact(&t, &conts[i]);
    }

    // Execute.
    kad_id_t         *id = &conts[4].id;
    kad_id_t         *ex = id;
    kad_contactlist_t contacts = {0};
    kad_table_find_closest(&t, id, ex, &contacts);
    unit_table_find_closest_check(&contacts);

    // Cleanup.
    kad_contactlist_fini(&contacts);
    kad_table_fini(&t);
    return MUNIT_OK;
}

MunitTest unit_table_tests[] = {
    {"/contains", unit_table_contains, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/add_contact", unit_table_add_contact, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/remove_contact", unit_table_remove_contact, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/find_closest", unit_table_find_closest, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
