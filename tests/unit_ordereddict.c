#include "ordereddict.h"
#include <munit.h>
#include <stdio.h>

MunitResult unit_ordereddict_insert(const MunitParameter params[], void *data)
{
    kad_ordereddict_t dict;
    kad_ordereddict_init(&dict);

    kad_uint256_t ids[] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0},
    };

    for (int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
    {
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_int(dict.size, ==, i + 1);
    }

    kad_ordereddict_fini(&dict);
    return MUNIT_OK;
}

MunitResult unit_ordereddict_insert_dup(const MunitParameter params[], void *data)
{
    kad_ordereddict_t dict;
    kad_ordereddict_init(&dict);

    kad_uint256_t ids[] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0},
    };
    int nids = sizeof(ids) / sizeof(ids[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_int(dict.size, ==, i + 1);
    }

    for (int i = 0; i < nids; i++)
    {
        // The size should not change.
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_int(dict.size, ==, nids);
    }

    kad_ordereddict_fini(&dict);
    return MUNIT_OK;
}

MunitResult unit_ordereddict_pop(const MunitParameter params[], void *data)
{
    kad_ordereddict_t dict;
    kad_ordereddict_init(&dict);

    // Setup.
    kad_uint256_t ids[] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0},
    };
    int nids = sizeof(ids) / sizeof(ids[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_int(dict.size, ==, i + 1);
    }

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        kad_contact_t popped;
        munit_assert_true(kad_ordereddict_pop(&dict, &ids[i], &popped));
        munit_assert_int(kad_uint256_cmp(&popped.id, &ids[i]), ==, 0);
        munit_assert_string_equal(popped.host, "127.0.0.1");
        munit_assert_int(popped.port, ==, 8080);
        munit_assert_int(dict.size, ==, nids - i - 1);
    }

    kad_ordereddict_fini(&dict);
    return MUNIT_OK;
}

MunitResult unit_ordereddict_pop_notexists(const MunitParameter params[], void *data)
{
    kad_ordereddict_t dict;
    kad_ordereddict_init(&dict);

    // Setup.
    kad_uint256_t ids[] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0},
    };
    int nids = sizeof(ids) / sizeof(ids[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_int(dict.size, ==, i + 1);
    }

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        munit_assert_true(kad_ordereddict_pop(&dict, &ids[i], NULL));
    }

    for (int i = 0; i < nids; i++)
    {
        // The size should not change.
        munit_assert_false(kad_ordereddict_pop(&dict, &ids[i], NULL));
        munit_assert_int(dict.size, ==, 0);
    }

    kad_ordereddict_fini(&dict);
    return MUNIT_OK;
}

MunitResult unit_ordereddict_contains(const MunitParameter params[], void *data)
{
    kad_ordereddict_t dict;
    kad_ordereddict_init(&dict);

    // Setup.
    kad_uint256_t ids[] = {
        {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0},
    };
    int nids = sizeof(ids) / sizeof(ids[0]);

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        munit_assert_false(kad_ordereddict_contains(&dict, &ids[i]));
        kad_ordereddict_insert(&dict, &(kad_contact_t){ids[i], "127.0.0.1", 8080});
        munit_assert_true(kad_ordereddict_contains(&dict, &ids[i]));
    }

    // Cleanup.
    kad_ordereddict_fini(&dict);
    return MUNIT_OK;
}

MunitTest unit_ordereddict_tests[] = {
    {"/insert", unit_ordereddict_insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/insert_dup", unit_ordereddict_insert_dup, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/pop", unit_ordereddict_pop, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/pop_notexists", unit_ordereddict_pop_notexists, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/contains", unit_ordereddict_contains, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
