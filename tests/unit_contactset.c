#include "crawlers/contactset.h"
#include <munit.h>

MunitResult unit_contactset_add(const MunitParameter params[], void *data)
{
    kad_contactset_t set;
    kad_contactset_init(&set);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 1, 1}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 2, 0, 2}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 3, 0, 0, 3}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 4, 0, 0, 0, 4}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 5, 0, 0, 0, 0, 5}, .host = "localhost", .port = 8080},
        {.id = {0, 6, 0, 0, 0, 0, 0, 6}, .host = "localhost", .port = 8080},
        {.id = {7, 0, 0, 0, 0, 0, 0, 7}, .host = "localhost", .port = 8080},
    };
    int nconts = sizeof(conts) / sizeof(conts[0]);

    // Execute.
    for (int i = 0; i < nconts; i++)
    {
        kad_contactset_add(&set, &conts[i]);
        munit_assert_int(set.size, ==, i + 1);
    }

    kad_contactset_fini(&set);
    return MUNIT_OK;
}

MunitResult unit_contactset_add_many(const MunitParameter params[], void *data)
{
    kad_contactset_t set;
    kad_contactset_init(&set);

    int nconts = 10000;

    // Execute.
    for (int i = 0; i < nconts; i++)
    {
        kad_contact_t cont = {
            .id = {rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()},
            .host = "localhost",
            .port = 8080,
        };

        // Add.
        kad_contactset_add(&set, &cont);
        munit_assert_int(set.size, ==, i + 1);
    }

    kad_contactset_fini(&set);
    return MUNIT_OK;
}

MunitResult unit_contactset_contains(const MunitParameter params[], void *data)
{
    kad_contactset_t set;
    kad_contactset_init(&set);

    int nconts = 1000;

    // Execute.
    for (int i = 0; i < nconts; i++)
    {
        kad_contact_t cont = {
            .id = {rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()},
            .host = "localhost",
            .port = 8080,
        };

        // Add.
        kad_contactset_add(&set, &cont);
        munit_assert_true(kad_contactset_contains(&set, &cont.id));
    }

    kad_contactset_fini(&set);
    return MUNIT_OK;
}

MunitTest unit_contactset_tests[] = {
    {"/add", unit_contactset_add, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/add_many", unit_contactset_add_many, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/contains", unit_contactset_contains, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
