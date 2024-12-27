#include "table.h"
#include "log.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct find_closest_user_s  find_closest_user_t;
typedef struct ping_callback_user_s ping_callback_user_t;

struct find_closest_user_s
{
    kad_id_t      *id;
    kad_id_t      *exclude;
    int            capacity;
    kad_contact_t *closest;
    kad_uint256_t *distances;
    int            nclosest;
};

struct ping_callback_user_s
{
    kad_bucket_t        *bucket;
    const kad_contact_t *old;
    const kad_contact_t *new;
};

static void kad_table_add_contact_inner(kad_table_t *s, const kad_contact_t *c, bool second);
static void kad_table_add_contact_inner_ping_callback(bool ok, void *user);

//
// Public
//

void kad_table_init(kad_table_t *s, kad_id_t *id, int capacity, kad_protocol_t *protocol)
{
    s->id = *id;
    s->capacity = capacity;
    s->buckets = malloc(sizeof(kad_bucket_t));
    assert(s->buckets && "Out of memory");

    kad_uint256_t lo = {0};
    kad_uint256_t hi = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
    kad_bucket_init(&s->buckets[0], &lo, &hi, capacity);

    s->nbuckets = 1;
    s->protocol = protocol;
}

void kad_table_fini(kad_table_t *s)
{
    for (int i = 0; i < s->nbuckets; i++)
    {
        kad_bucket_fini(&s->buckets[i]);
    }

    free(s->buckets);
    memset(s, 0, sizeof(*s));
}

bool kad_table_contains(const kad_table_t *s, kad_id_t *id)
{
    for (int bix = 0; bix < s->nbuckets; bix++)
    {
        if (kad_bucket_contains(&s->buckets[bix], id))
        {
            return true;
        }
    }

    return false;
}

void kad_table_iter_expired_buckets(kad_table_t *s, void (*cb)(kad_bucket_t *b, void *data), void *data)
{
    for (int bix = 0; bix < s->nbuckets; bix++)
    {
        if (kad_bucket_is_expired(&s->buckets[bix]))
        {
            cb(&s->buckets[bix], data);
        }
    }
}

int kad_table_get_bucket_index(const kad_table_t *s, kad_id_t *id)
{
    for (int bix = 0; bix < s->nbuckets; bix++)
    {
        bool above = kad_uint256_cmp(id, &s->buckets[bix].range_lower) >= 0;
        bool below = kad_uint256_cmp(id, &s->buckets[bix].range_upper) <= 0;
        if (above && below)
        {
            return bix;
        }
    }

    kad_error("Identifier does not fall into any bucket.\n");
    kad_error("  id:       %U\n", id);
    kad_error("  nbuckets: %d\n", s->nbuckets);
    abort();
}

void kad_table_split_bucket(kad_table_t *s, int bix)
{
    // Allocate space at index + 1.
    s->nbuckets++;
    s->buckets = realloc(s->buckets, s->nbuckets * sizeof(s->buckets[0]));
    assert(s->buckets && "Out of memory");

    // s->nbuckets = 3, bix = 1. s->nbuckets - bix - 2 = 0.
    // s->nbuckets = 3, bix = 0. s->nbuckets - bix - 2 = 1.
    memmove(&s->buckets[bix + 2], &s->buckets[bix + 1], (s->nbuckets - bix - 2) * sizeof(s->buckets[0]));
    kad_bucket_init(&s->buckets[bix + 1], NULL, NULL, s->capacity);
    kad_bucket_split(&s->buckets[bix], &s->buckets[bix + 1]);
}

void kad_table_add_contact(kad_table_t *s, const kad_contact_t *c)
{
    kad_table_add_contact_inner(s, c, false);
}

bool kad_table_can_split_bucket(const kad_table_t *s, const kad_bucket_t *b)
{
    bool above = kad_uint256_cmp(&s->id, &b->range_lower);
    bool below = kad_uint256_cmp(&s->id, &b->range_upper);
    return (above && below) || (kad_bucket_depth(b) % 5);
}

void kad_table_remove_contact(const kad_table_t *s, kad_id_t *id)
{
    int           ix = kad_table_get_bucket_index(s, id);
    kad_bucket_t *bucket = &s->buckets[ix];
    kad_bucket_remove_contact(bucket, id);
}

int kad_table_traverse_buckets(const kad_table_t *s, int bix, int (*cb)(const kad_contact_t *c, void *data), void *data)
{
    if (kad_bucket_iter(&s->buckets[bix], cb, data) == ITER_STOP)
    {
        return ITER_STOP;
    }

    // bix - 1.
    // bix + 1.
    // bix - 2.
    // bix + 2.
    for (int offset = 1;; offset++)
    {
        int west = bix - offset;
        int east = bix + offset;

        bool is_west_in_range = west >= 0;
        bool is_east_in_range = east < s->nbuckets;

        if (is_west_in_range)
        {
            if (kad_bucket_iter(&s->buckets[west], cb, data) == ITER_STOP)
            {
                return ITER_STOP;
            }
        }

        if (is_east_in_range)
        {
            if (kad_bucket_iter(&s->buckets[east], cb, data) == ITER_STOP)
            {
                return ITER_STOP;
            }
        }

        if (!is_west_in_range && !is_east_in_range)
        {
            break;
        }
    }

    return ITER_CONT;
}

int kad_table_find_closest_inner(const kad_contact_t *c, void *data)
{
    find_closest_user_t *user = (find_closest_user_t *)(data);
    if (kad_uint256_cmp(&c->id, user->exclude) != 0)
    {
        kad_uint256_t dist;
        kad_uint256_xor(&c->id, user->id, &dist);

        // Insert into list in sorted order.
        int l = 0;
        int r = user->nclosest;
        while (l < r)
        {
            int mid = (l + r) / 2;
            int cmp = kad_uint256_cmp(&dist, &user->distances[mid]);
            if (cmp < 0)
            {
                // dist < ctx->dists[mid].
                r = mid;
            }
            else
            {
                // dist >= ctx->dists[mid].
                l = mid + 1;
            }
        }

        // Insert here.
        memmove(&user->closest[l + 1], &user->closest[l], (user->nclosest - l) * sizeof(user->closest[0]));
        memmove(&user->distances[l + 1], &user->distances[l], (user->nclosest - l) * sizeof(user->distances[0]));
        user->nclosest++;
        user->closest[l] = *c;
        user->distances[l] = dist;
    }

    if (user->nclosest >= user->capacity)
    {
        // We've found enough contacts.
        return ITER_STOP;
    }
    else
    {
        return ITER_CONT;
    }
}

void kad_table_find_closest(const kad_table_t *s, kad_id_t *id, kad_id_t *exclude,
                            void (*cb)(const kad_contact_t *c, void *data), void *data)
{
    find_closest_user_t user = {.id = id, .exclude = exclude, .capacity = s->capacity};
    user.closest = malloc(s->capacity * sizeof(user.closest[0]));
    user.distances = malloc(s->capacity * sizeof(user.distances[0]));
    assert(user.closest && user.distances && "Out of memory");

    // Execute.
    int bix = kad_table_get_bucket_index(s, id);
    kad_table_traverse_buckets(s, bix, kad_table_find_closest_inner, &user);
    for (int i = 0; i < user.nclosest; i++)
    {
        cb(&user.closest[i], data);
    }

    // Cleanup.
    free(user.closest);
    free(user.distances);
}

//
// Static
//

void kad_table_add_contact_inner(kad_table_t *s, const kad_contact_t *c, bool second)
{
    int           ix = kad_table_get_bucket_index(s, &c->id);
    kad_bucket_t *bucket = &s->buckets[ix];
    kad_bucket_touch(bucket);

    if (kad_bucket_add_contact(bucket, c) || second)
    {
        // If we've successfully added the contact, or if this is the second time we are trying to do so, return.
        return;
    }

    if (kad_table_can_split_bucket(s, bucket))
    {
        kad_table_split_bucket(s, ix);
        kad_table_add_contact_inner(s, c, true);
    }
    else
    {
        // The bucket is full and we can't split it. We need to issue a task to ping the oldest node in the bucket, and
        // replace it if that ping fails.
        if (s->protocol)
        {
            kad_contact_t head;
            kad_bucket_head(bucket, &head);
            s->protocol->ping(&(kad_ping_args_t){
                .self = s->protocol,
                .id = &s->id,
                .host = head.host,
                .port = head.port,
                .callback = kad_table_add_contact_inner_ping_callback,
                .user = &(ping_callback_user_t){.bucket = bucket, .old = &head, .new = c},
            });
        }
    }
}

void kad_table_add_contact_inner_ping_callback(bool ok, void *user)
{
    if (ok)
    {
        // Do nothing.
    }
    else
    {
        ping_callback_user_t *user = (ping_callback_user_t *)(user);
        kad_bucket_remove_contact(user->bucket, &user->old->id);
        kad_bucket_add_contact(user->bucket, user->new);
        kad_bucket_touch(user->bucket);
    }
}
