#ifndef KADEMLIA_ORDEREDDICT_H
#define KADEMLIA_ORDEREDDICT_H

#include <kademlia/contact.h>
#include <stdbool.h>

typedef struct kad_ordereddictnode_s kad_ordereddictnode_t;
typedef struct kad_ordereddict_s kad_ordereddict_t;

struct kad_ordereddictnode_s
{
    kad_contact_t c;
    kad_ordereddictnode_t *next;
};

struct kad_ordereddict_s
{
    // TODO: Radix Tree.
    kad_ordereddictnode_t *head;
    int size;
};

void kad_ordereddict_init(kad_ordereddict_t *s);
void kad_ordereddict_fini(kad_ordereddict_t *s);
kad_ordereddict_t *kad_ordereddict_new(void);
void kad_ordereddict_free(kad_ordereddict_t *s);
void kad_ordereddict_insert(kad_ordereddict_t *s, kad_contact_t *c);
bool kad_ordereddict_pop(kad_ordereddict_t *s, const kad_uint256_t *id, kad_contact_t *out);
bool kad_ordereddict_popfront(kad_ordereddict_t *s, kad_contact_t *out);
bool kad_ordereddict_popback(kad_ordereddict_t *s, kad_contact_t *out);

#endif // KADEMLIA_ORDEREDDICT_H
