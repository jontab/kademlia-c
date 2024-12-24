#include "bucket.h"
#include "log.h"
#include <munit.h>

#define MIN(A, B) (((A) < (B)) ? (A) : (B))

MunitResult unit_bucket_add_contact(const MunitParameter params[], void *data)
{
    // Setup.
    kad_uint256_t lo = {0, 0, 0, 0, 0, 0, 0, 0};
    kad_uint256_t hi = {0, 0, 0, 0, 0, 0, 0, 0xffffffff};
    kad_bucket_t *b = kad_bucket_new(&lo, &hi, 1);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 2}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 3}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 4}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 5}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 6}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 7}, .host = "localhost", .port = "8080"},
    };

    // Execute.
    for (int i = 0; i < sizeof(conts) / sizeof(conts[0]); i++)
    {
        if (i == 0)
        {
            munit_assert_true(kad_bucket_add_contact(b, &conts[i]));
            munit_assert_int(b->contacts.size, ==, 1);
            munit_assert_int(b->replacements.size, ==, 0);
        }
        else
        {
            munit_assert_false(kad_bucket_add_contact(b, &conts[i]));
            munit_assert_int(b->contacts.size, ==, 1);
            munit_assert_int(b->replacements.size, ==, MIN(i, 4));
        }
    }

    // Cleanup.
    kad_bucket_free(b);
    return MUNIT_OK;
}

MunitResult unit_bucket_remove_contact(const MunitParameter params[], void *data)
{
    // Setup.
    kad_uint256_t lo = {0, 0, 0, 0, 0, 0, 0, 0};
    kad_uint256_t hi = {0, 0, 0, 0, 0, 0, 0, 0xffffffff};
    kad_bucket_t *b = kad_bucket_new(&lo, &hi, 1);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 2}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 3}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 4}, .host = "localhost", .port = "8080"},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_bucket_add_contact(b, &conts[i]);
    }

    // Execute.
    for (int i = 0; i < nids; i++)
    {
        kad_bucket_remove_contact(b, &conts[i].id);
        munit_assert_int(b->contacts.size, ==, (i < nids - 1) ? 1 : 0);
        munit_assert_int(b->replacements.size, ==, (i < nids - 1) ? nids - i - 2 : 0);
    }

    // Cleanup.
    kad_bucket_free(b);
    return MUNIT_OK;
}

MunitResult unit_bucket_depth(const MunitParameter params[], void *data)
{
    // Setup.
    kad_uint256_t lo = {0, 0, 0, 0, 0, 0, 0, 0};
    kad_uint256_t hi = {0, 0, 0, 0, 0, 0, 0, 0xffffffff};
    kad_bucket_t *b = kad_bucket_new(&lo, &hi, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0x00, 0, 0x00, 0, 0x00, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0x00, 0, 0x00, 0, 0x10, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0x00, 0, 0x10, 0, 0x00, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0x10, 0, 0x00, 0, 0x00, 0, 0}, .host = "localhost", .port = "8080"},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    // Execute.
    munit_assert_int(kad_bucket_depth(b), ==, 0);
    kad_bucket_add_contact(b, &conts[0]);
    munit_assert_int(kad_bucket_depth(b), ==, 256);
    kad_bucket_add_contact(b, &conts[1]);
    munit_assert_int(kad_bucket_depth(b), ==, 187);
    kad_bucket_add_contact(b, &conts[2]);
    munit_assert_int(kad_bucket_depth(b), ==, 123);
    kad_bucket_add_contact(b, &conts[3]);
    munit_assert_int(kad_bucket_depth(b), ==, 59);

    // Cleanup.
    kad_bucket_free(b);
    return MUNIT_OK;
}

MunitResult unit_bucket_split(const MunitParameter params[], void *data)
{
    // Setup.
    kad_uint256_t lo = {0, 0, 0, 0, 0, 0, 0, 0};
    kad_uint256_t hi = {0, 0, 0, 0, 0, 0, 0, 0xffffffff};
    kad_uint256_t ml = {0, 0, 0, 0, 0, 0, 0, 0x7fffffff};
    kad_uint256_t mr = {0, 0, 0, 0, 0, 0, 0, 0x80000000};
    kad_bucket_t *b = kad_bucket_new(&lo, &hi, 8);
    kad_bucket_t *r = kad_bucket_new(NULL, NULL, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 1}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 2}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 3}, .host = "localhost", .port = "8080"},
        {.id = {0, 0, 0, 0, 0, 0, 0, 4}, .host = "localhost", .port = "8080"},
    };
    int nids = sizeof(conts) / sizeof(conts[0]);

    for (int i = 0; i < nids; i++)
    {
        kad_bucket_add_contact(b, &conts[i]);
    }

    // Execute.
    kad_bucket_split(b, r);
    munit_assert_int(kad_uint256_cmp(&b->range_lower, &lo), ==, 0);
    munit_assert_int(kad_uint256_cmp(&b->range_upper, &ml), ==, 0);
    munit_assert_int(kad_uint256_cmp(&r->range_lower, &mr), ==, 0);
    munit_assert_int(kad_uint256_cmp(&r->range_upper, &hi), ==, 0);
    munit_assert_int(b->contacts.size, ==, nids);
    munit_assert_int(r->contacts.size, ==, 0);
    munit_assert_int(b->replacements.size, ==, 0);
    munit_assert_int(r->replacements.size, ==, 0);

    // Cleanup.
    kad_bucket_free(b);
    kad_bucket_free(r);
    return MUNIT_OK;
}

MunitTest unit_bucket_tests[] = {
    {"/add_contact", unit_bucket_add_contact, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/remove_contact", unit_bucket_remove_contact, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/depth", unit_bucket_depth, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/split", unit_bucket_split, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
