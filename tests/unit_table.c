#include "log.h"
#include "table.h"
#include "uint256.h"
#include <munit.h>

MunitResult unit_table_contains(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){0}, 8, NULL);

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
    kad_table_init(&t, &(kad_uint256_t){rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()}, 8, NULL);

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
    kad_table_init(&t, &(kad_uint256_t){0}, 8, NULL);

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

void unit_table_find_closest_cb(const kad_contact_t *c, void *data)
{
    int *i = (int *)(intptr_t)(data);
    switch (*i)
    {
    case 0: {
        kad_id_t id = {{0x80000000, 0, 0, 0, 0, 0, 2, 0}};
        munit_assert_int(kad_uint256_cmp(&c->id, &id), ==, 0);
        (*i)++;
        break;
    }

    case 1: {
        kad_id_t id = {{0x80000000, 0, 0, 0, 0, 3, 0, 0}};
        munit_assert_int(kad_uint256_cmp(&c->id, &id), ==, 0);
        (*i)++;
        break;
    }

    case 2: {
        kad_id_t id = {{0x80000000, 0, 0, 0, 4, 0, 0, 0}};
        munit_assert_int(kad_uint256_cmp(&c->id, &id), ==, 0);
        (*i)++;
        break;
    }

    case 3: {
        kad_id_t id = {{0x00000000, 0, 0, 0, 0, 0, 0, 1}};
        munit_assert_int(kad_uint256_cmp(&c->id, &id), ==, 0);
        (*i)++;
        break;
    }

    default:
        munit_assert_true(false); // There should only be 4 contacts.
    }
}

MunitResult unit_table_find_closest(const MunitParameter params[], void *data)
{
    // Setup.
    kad_table_t t;
    kad_table_init(&t, &(kad_uint256_t){0}, 4, NULL);

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
    int       i = 0;
    kad_id_t *id = &conts[4].id;
    kad_id_t *ex = id;
    kad_table_find_closest(&t, id, ex, unit_table_find_closest_cb, (void *)(&i));

    // Cleanup.
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
