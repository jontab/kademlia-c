#include "crawlers/contactheap.h"
#include <munit.h>

static void unit_contactheap_unmarked_cb(const kad_contact_t *c, void *user);

MunitResult unit_contactheap_contains(const MunitParameter params[], void *data)
{
    // Setup.
    kad_contactheap_t heap;
    kad_contactheap_init(&heap, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);

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

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_push(&heap, &conts[i]);
    }

    // Execute.
    for (int i = 0; i < nconts; i++)
    {
        munit_assert_true(kad_contactheap_contains(&heap, &conts[i].id));
    }

    // Cleanup.
    kad_contactheap_fini(&heap);
    return MUNIT_OK;
}

MunitResult unit_contactheap_unmarked(const MunitParameter params[], void *data)
{
    // Setup.
    kad_contactheap_t heap;
    kad_contactheap_init(&heap, &(kad_id_t){{0, 0, 0, 0xffffffff, 0, 0, 0, 0}}, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0xffffffff, 0, 0, 0, 0x0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0xffffffff, 0, 0, 0, 0x1}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0xffffffff, 0, 0, 0, 0xf}, .host = "localhost", .port = 8080}, // Unmarked.
        {.id = {0, 0, 0, 0xffffffff, 0, 0, 0, 0x2}, .host = "localhost", .port = 8080}, // Unmarked (closer).
    };
    int nconts = sizeof(conts) / sizeof(conts[0]);

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_push(&heap, &conts[i]);
    }

    kad_contactheap_mark(&heap, &conts[0].id);
    kad_contactheap_mark(&heap, &conts[1].id);

    // Execute.
    int i = 0;
    kad_contactheap_unmarked(&heap, unit_contactheap_unmarked_cb, &i);

    // Cleanup.
    kad_contactheap_fini(&heap);
    return MUNIT_OK;
}

MunitResult unit_contactheap_done(const MunitParameter params[], void *data)
{
    // Setup.
    kad_contactheap_t heap;
    kad_contactheap_init(&heap, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);

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

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_push(&heap, &conts[i]);
    }

    // Execute.
    munit_assert_false(kad_contactheap_done(&heap));

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_mark(&heap, &conts[i].id);
    }

    munit_assert_true(kad_contactheap_done(&heap));

    // Cleanup.
    kad_contactheap_fini(&heap);
    return MUNIT_OK;
}

MunitResult unit_contactheap_remove(const MunitParameter params[], void *data)
{
    // Setup.
    kad_contactheap_t heap;
    kad_contactheap_init(&heap, &(kad_id_t){{1, 2, 3, 4, 5, 6, 7, 8}}, 8);

    kad_contact_t conts[] = {
        {.id = {0, 0, 0, 0, 0, 0, 0, 0}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 0, 1, 1}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 0, 2, 0, 2}, .host = "localhost", .port = 8080},
        {.id = {0, 0, 0, 0, 3, 0, 0, 3}, .host = "localhost", .port = 8080},
    };
    int nconts = sizeof(conts) / sizeof(conts[0]);

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_push(&heap, &conts[i]);
    }

    // Execute.
    munit_assert_false(kad_contactheap_done(&heap));

    for (int i = 0; i < nconts; i++)
    {
        kad_contactheap_remove(&heap, &conts[i].id);
    }

    munit_assert_true(kad_contactheap_done(&heap));

    // Cleanup.
    kad_contactheap_fini(&heap);
    return MUNIT_OK;
}

MunitTest unit_contactheap_tests[] = {
    {"/contains", unit_contactheap_contains, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/unmarked", unit_contactheap_unmarked, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/done", unit_contactheap_done, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/remove", unit_contactheap_remove, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

//
// Statics
//

void unit_contactheap_unmarked_cb(const kad_contact_t *c, void *user)
{
    int *i = (int *)(user);
    if (*i == 0)
    {
        // This contact will be first because it is XOR-closer to the target than the other contact.
        munit_assert_int(kad_uint256_cmp(&c->id, &(kad_id_t){{0, 0, 0, 0xffffffff, 0, 0, 0, 0x2}}), ==, 0);
    }
    else
    {
        munit_assert_int(kad_uint256_cmp(&c->id, &(kad_id_t){{0, 0, 0, 0xffffffff, 0, 0, 0, 0xf}}), ==, 0);
    }

    (*i)++;
}
