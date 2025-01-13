#ifndef KADEMLIA_NODECRAWLER_H
#define KADEMLIA_NODECRAWLER_H

#include "../protocol.h"
#include "../uint256.h"
#include "contactheap.h"

//
// Typedefs
//

typedef struct kad_nodecrawler_s     kad_nodecrawler_t;
typedef struct kad_nodecrawlerargs_s kad_nodecrawlerargs_t;

typedef void (*kad_nodecrawlercb_t)(const kad_contact_t *array, int size, void *user);

//
// Structs
//

struct kad_nodecrawler_s
{
    kad_id_t          id;
    kad_id_t          target;
    int               capacity;
    int               alpha;
    kad_contactheap_t nearest;
    kad_protocol_t   *proto;
    kad_contactset_t  ignore;
};

struct kad_nodecrawlerargs_s
{
    kad_id_t             id;
    kad_id_t             target;
    int                  capacity;
    int                  alpha;
    const kad_contact_t *contacts;      // To initialize heap.
    int                  contacts_size; // To initialize heap.
    kad_protocol_t      *proto;
};

//
// Methods
//

void kad_nodecrawler_init(kad_nodecrawler_t *s, const kad_nodecrawlerargs_t *args);
void kad_nodecrawler_fini(kad_nodecrawler_t *s);
void kad_nodecrawler_find(kad_nodecrawler_t *s, kad_nodecrawlercb_t cb, void *user);

#endif // KADEMLIA_NODECRAWLER_H
