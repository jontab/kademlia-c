#ifndef KADEMLIA_TABLE_H
#define KADEMLIA_TABLE_H

#include "bucket.h"
#include "uint256.h"

//
// Typedefs
//

typedef struct kad_table_s kad_table_t;

//
// Structs
//

struct kad_table_s
{
    kad_uint256_t          id;
    int                    capacity;
    kad_bucket_t          *buckets;
    int                    nbuckets;
    struct kad_protocol_s *protocol;
};

//
// Methods
//

void kad_table_init(kad_table_t *s, const kad_id_t *id, int capacity);
void kad_table_fini(kad_table_t *s);
void kad_table_set_protocol(kad_table_t *s, struct kad_protocol_s *protocol);
bool kad_table_contains(const kad_table_t *s, const kad_id_t *id);
void kad_table_iter_expired_buckets(kad_table_t *s, void (*cb)(kad_bucket_t *b, void *data), void *data);
int  kad_table_get_bucket_index(const kad_table_t *s, const kad_id_t *id);
void kad_table_split_bucket(kad_table_t *s, int bix);
void kad_table_add_contact(kad_table_t *s, const kad_contact_t *c);
bool kad_table_can_split_bucket(const kad_table_t *s, const kad_bucket_t *b);
void kad_table_remove_contact(const kad_table_t *s, const kad_id_t *id);
int  kad_table_traverse_buckets(const kad_table_t *s, int bix, int (*cb)(const kad_contact_t *c, void *data),
                                void *data);
void kad_table_find_closest(const kad_table_t *s, const kad_id_t *id, const kad_id_t *exclude,
                            kad_contact_t **contacts, int *contacts_size);

#endif // KADEMLIA_TABLE_H
