#ifndef KADEMLIA_BUCKET_H
#define KADEMLIA_BUCKET_H

#include "ordereddict.h"
#include "uint256.h"
#include <stdbool.h>
#include <time.h>

typedef struct kad_bucket_s kad_bucket_t;

struct kad_bucket_s
{
    kad_uint256_t range_lower;
    kad_uint256_t range_upper;
    int capacity;
    kad_ordereddict_t contacts;
    kad_ordereddict_t replacements;
    struct timespec last_touched_at;
};

void kad_bucket_init(kad_bucket_t *s, kad_id_t *range_lower, kad_id_t *range_upper, int capacity);
void kad_bucket_fini(kad_bucket_t *s);
kad_bucket_t *kad_bucket_new(kad_id_t *range_lower, kad_id_t *range_upper, int capacity);
void kad_bucket_free(kad_bucket_t *s);
void kad_bucket_touch(kad_bucket_t *s);
bool kad_bucket_add_contact(kad_bucket_t *s, const kad_contact_t *c);
void kad_bucket_add_replacement(kad_bucket_t *s, const kad_contact_t *c);
void kad_bucket_remove_contact(kad_bucket_t *s, kad_id_t *id);
int kad_bucket_depth(const kad_bucket_t *s);
void kad_bucket_split(kad_bucket_t *s, kad_bucket_t *r);
bool kad_bucket_is_full(const kad_bucket_t *s);
bool kad_bucket_is_expired(const kad_bucket_t *s);
int kad_bucket_iter(const kad_bucket_t *s, int (*cb)(const kad_contact_t *c, void *data), void *data);
bool kad_bucket_contains(const kad_bucket_t *s, kad_id_t *id);

#endif // KADEMLIA_BUCKET_H
