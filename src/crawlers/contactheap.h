#ifndef KAD_CONTACTHEAP_H
#define KAD_CONTACTHEAP_H

#include "../contact.h"
#include "../uint256.h"
#include "../ds/contactset.h"
#include "../ds/contactlist.h"

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct kad_contactheap_s kad_contactheap_t;

typedef void (*kad_contactheapiter_t)(const kad_contact_t *c, void *user);
typedef void (*kad_contactheapunmarkediter_t)(const kad_contact_t *c, void *user);

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct kad_contactheap_s
{
    kad_id_t         target;
    int              capacity;
    kad_contactset_t marked;
    kad_uint256_t   *heap_distances;
    kad_contact_t   *heap_contacts;
    int              heap_size;
};

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_contactheap_init(kad_contactheap_t *s, const kad_id_t *target, int capacity);
void kad_contactheap_fini(kad_contactheap_t *s);
bool kad_contactheap_contains(const kad_contactheap_t *s, const kad_id_t *id);
void kad_contactheap_iter(const kad_contactheap_t *s, kad_contactheapiter_t iter, void *user);
void kad_contactheap_push(kad_contactheap_t *s, const kad_contact_t *c);
void kad_contactheap_mark(kad_contactheap_t *s, const kad_id_t *id);
void kad_contactheap_unmarked(const kad_contactheap_t *s, kad_contactheapunmarkediter_t iter, void *user);
bool kad_contactheap_done(const kad_contactheap_t *s);
void kad_contactheap_remove(kad_contactheap_t *s, const kad_id_t *id);

#endif // KAD_CONTACTHEAP_H
