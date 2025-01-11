#ifndef KADEMLIA_VALUECRAWLER_H
#define KADEMLIA_VALUECRAWLER_H

#include "../protocol.h"
#include "../uint256.h"
#include "contactheap.h"

//
// Typedefs
//

typedef struct kad_valuecrawler_s     kad_valuecrawler_t;
typedef struct kad_valuecrawlerargs_s kad_valuecrawlerargs_t;

typedef void (*kad_valuecrawlercb_t)(const char *value, void *user);

//
// Structs
//

struct kad_valuecrawler_s
{
    kad_id_t          id;
    const char       *key;
    int               capacity;
    int               alpha;
    kad_contactheap_t nearest;
    kad_protocol_t   *proto;
    kad_contactset_t  ignore;
};

struct kad_valuecrawlerargs_s
{
    kad_id_t             id;
    const char          *key;
    int                  capacity;
    int                  alpha;
    const kad_contact_t *contacts;
    int                  contacts_size;
    kad_protocol_t      *proto;
};

//
// Methods
//

void kad_valuecrawler_init(kad_valuecrawler_t *s, const kad_valuecrawlerargs_t *args);
void kad_valuecrawler_fini(kad_valuecrawler_t *s);
void kad_valuecrawler_find(kad_valuecrawler_t *s, kad_valuecrawlercb_t cb, void *user);

#endif // KADEMLIA_VALUECRAWLER_H
