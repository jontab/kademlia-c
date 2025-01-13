#include "bucket.h"
#include "alloc.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_bucket_init(kad_bucket_t *s, const kad_id_t *range_lower, const kad_id_t *range_upper, int capacity)
{
    s->range_lower = range_lower ? *range_lower : (kad_uint256_t){0}; // Inclusive.
    s->range_upper = range_upper ? *range_upper : (kad_uint256_t){0}; // Inclusive.
    s->capacity = capacity;
    kad_ordereddict_init(&s->contacts);
    kad_ordereddict_init(&s->replacements);
    kad_bucket_touch(s); // last_touched_at.
}

void kad_bucket_fini(kad_bucket_t *s)
{
    kad_ordereddict_fini(&s->contacts);
    kad_ordereddict_fini(&s->replacements);
}

kad_bucket_t *kad_bucket_new(const kad_id_t *range_lower, const kad_id_t *range_upper, int capacity)
{
    kad_bucket_t *s = kad_alloc(1, sizeof(kad_bucket_t));
    kad_bucket_init(s, range_lower, range_upper, capacity);
    return s;
}

void kad_bucket_free(kad_bucket_t *s)
{
    kad_bucket_fini(s);
    free(s);
}

void kad_bucket_touch(kad_bucket_t *s)
{
    clock_gettime(CLOCK_MONOTONIC_RAW, &s->last_touched_at);
}

bool kad_bucket_add_contact(kad_bucket_t *s, const kad_contact_t *c)
{
    if (kad_ordereddict_pop(&s->contacts, &c->id, NULL) || !kad_bucket_is_full(s))
    {
        kad_ordereddict_insert(&s->contacts, c);
        return true;
    }
    else
    {
        kad_bucket_add_replacement(s, c);
        return false;
    }
}

/**
 * @param c Contact. Copies.
 */
void kad_bucket_add_replacement(kad_bucket_t *s, const kad_contact_t *c)
{
    kad_ordereddict_pop(&s->replacements, &c->id, NULL);
    kad_ordereddict_insert(&s->replacements, c);
    while (s->replacements.size >= 5 * s->capacity)
    {
        kad_ordereddict_popfront(&s->replacements, NULL);
    }
}

void kad_bucket_remove_contact(kad_bucket_t *s, const kad_id_t *id)
{
    kad_ordereddict_pop(&s->replacements, id, NULL);
    if (kad_ordereddict_pop(&s->contacts, id, NULL))
    {
        // Promote newest replacement to tail of contact list.
        kad_contact_t promoted;
        if (kad_ordereddict_popback(&s->replacements, &promoted))
        {
            kad_ordereddict_insert(&s->contacts, &promoted);
            kad_debug("Promoted contact from replacements to contact list\n");
            kad_debug("%C\n", &promoted);
        }
    }
}

int kad_bucket_depth(const kad_bucket_t *s)
{
    if (s->contacts.size == 0)
    {
        return 0;
    }

    int intix = 0;
    int bitix = 0;

    int nints = sizeof(s->contacts.head->c.id.d) / sizeof(s->contacts.head->c.id.d[0]);
    int nbits = sizeof(s->contacts.head->c.id.d[0]) * 8;

    int count = 0;
    while (intix < nints)
    {
        // TODO: Make an iterator and abstract all of this away.
        int mask = 1 << (nbits - bitix - 1);
        int bit1 = (mask & s->contacts.head->c.id.d[intix]) >> (nbits - bitix - 1);
        int same = 1;

        kad_ordereddictnode_t *curr = s->contacts.head->next;
        while (curr)
        {
            int curr_bit = (mask & curr->c.id.d[intix]) >> (nbits - bitix - 1);
            if (curr_bit == bit1)
            {
                curr = curr->next;
            }
            else
            {
                same = 0;
                break;
            }
        }

        if (same)
        {
            count++;
            bitix++;

            intix += bitix / nbits;
            bitix %= nbits;
        }
        else
        {
            break;
        }
    }

    return count;
}

void kad_bucket_split(kad_bucket_t *s, kad_bucket_t *r)
{
    assert((r->contacts.size == 0) && "Expected new bucket");
    assert((r->replacements.size == 0) && "Expected new bucket");

    kad_uint256_t midl;
    kad_uint256_t midr;
    kad_uint256_avg(&s->range_lower, &s->range_upper, &midl);
    kad_uint256_add(&midl, &(kad_uint256_t){0, 0, 0, 0, 0, 0, 0, 1}, &midr);

    r->range_upper = s->range_upper;
    r->range_lower = midr;
    s->range_upper = midl;

    // 1. Loop through all contacts.
    // 2. Loop through all replacements.
    kad_contact_t     cont;
    kad_ordereddict_t all;
    kad_ordereddict_init(&all);
    while (kad_ordereddict_popfront(&s->contacts, &cont))
    {
        kad_ordereddict_insert(&all, &cont);
    }

    while (kad_ordereddict_popfront(&s->replacements, &cont))
    {
        kad_ordereddict_insert(&all, &cont);
    }

    // 3. Loop through everything and partition them correctly.
    while (kad_ordereddict_popfront(&all, &cont))
    {
        if (kad_uint256_cmp(&cont.id, &midl) <= 0)
        {
            kad_bucket_add_contact(s, &cont);
        }
        else
        {
            kad_bucket_add_contact(r, &cont);
        }
    }

    kad_ordereddict_fini(&all);
}

bool kad_bucket_is_full(const kad_bucket_t *s)
{
    return s->contacts.size >= s->capacity;
}

bool kad_bucket_is_expired(const kad_bucket_t *s)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return (now.tv_sec - s->last_touched_at.tv_sec) > 60; // Minute.
}

int kad_bucket_iter(const kad_bucket_t *s, int (*cb)(const kad_contact_t *c, void *data), void *data)
{
    return kad_ordereddict_iter(&s->contacts, cb, data);
}

bool kad_bucket_contains(const kad_bucket_t *s, const kad_id_t *id)
{
    return kad_ordereddict_contains(&s->contacts, id);
}

void kad_bucket_head(const kad_bucket_t *s, kad_contact_t *c)
{
    *c = s->contacts.head->c;
}
