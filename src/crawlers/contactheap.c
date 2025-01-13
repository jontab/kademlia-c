#include "contactheap.h"
#include "../alloc.h"
#include <stdlib.h>
#include <string.h>

#define MIN(A, B) (((A) < (B)) ? (A) : (B))

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct unmarked_context_s unmarked_context_t;

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct unmarked_context_s
{
    const kad_contactheap_t      *s;
    kad_contactheapunmarkediter_t iter;
    void                         *user;
};

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_contactheap_init(kad_contactheap_t *s, const kad_id_t *target, int capacity)
{
    memset(s, 0, sizeof(*s));
    s->target = *target;
    s->capacity = capacity;
    kad_contactset_init(&s->marked);
}

void kad_contactheap_fini(kad_contactheap_t *s)
{
    if (s->heap_distances)
    {
        free(s->heap_distances);
        free(s->heap_contacts);
    }

    kad_contactset_fini(&s->marked);
    memset(s, 0, sizeof(*s));
}

bool kad_contactheap_contains(const kad_contactheap_t *s, const kad_id_t *id)
{
    for (int i = 0; i < s->heap_size; i++)
    {
        const kad_contact_t *heap_contact = &s->heap_contacts[i];
        const kad_id_t      *heap_id = &heap_contact->id;
        if (kad_uint256_cmp(heap_id, id) == 0)
        {
            return true;
        }
    }

    return false;
}

void kad_contactheap_iter(const kad_contactheap_t *s, kad_contactheapiter_t iter, void *user)
{
    for (int i = 0; i < MIN(s->capacity, s->heap_size); i++)
    {
        iter(&s->heap_contacts[i], user);
    }
}

void kad_contactheap_push(kad_contactheap_t *s, const kad_contact_t *c)
{
    if (!kad_contactheap_contains(s, &c->id))
    {
        s->heap_contacts = kad_realloc(s->heap_contacts, (s->heap_size + 1) * sizeof(s->heap_contacts[0]));
        s->heap_distances = kad_realloc(s->heap_distances, (s->heap_size + 1) * sizeof(s->heap_distances[0]));

        kad_uint256_t distance;
        kad_uint256_xor(&c->id, &s->target, &distance);

        int l = 0;
        int r = s->heap_size;
        while (l < r)
        {
            int heap_mid = (l + r) / 2;
            int heap_cmp = kad_uint256_cmp(&distance, &s->heap_distances[heap_mid]);
            if (heap_cmp < 0)
            {
                // distance < heap_distances[heap_mid];
                r = heap_mid;
            }
            else
            {
                // distance >= heap_distances[heap_mid];
                l = heap_mid + 1;
            }
        }

        // Insert.
        memmove(&s->heap_contacts[l + 1], &s->heap_contacts[l], (s->heap_size - l) * sizeof(s->heap_contacts[0]));
        memmove(&s->heap_distances[l + 1], &s->heap_distances[l], (s->heap_size - l) * sizeof(s->heap_distances[0]));
        s->heap_size++;
        s->heap_contacts[l] = *c;
        s->heap_distances[l] = distance;
    }
}

void kad_contactheap_mark(kad_contactheap_t *s, const kad_id_t *id)
{
    kad_contact_t c = {.id = *id, .host = {0}, .port = 0};
    kad_contactset_add(&s->marked, &c);
}

void kad_contactheap_unmarked_cb(const kad_contact_t *c, void *user)
{
    unmarked_context_t *context = (unmarked_context_t *)(user);
    if (!kad_contactset_contains(&context->s->marked, &c->id))
    {
        context->iter(c, context->user);
    }
}

void kad_contactheap_unmarked(const kad_contactheap_t *s, kad_contactheapunmarkediter_t iter, void *user)
{
    unmarked_context_t context = {.s = s, .iter = iter, .user = user};
    kad_contactheap_iter(s, kad_contactheap_unmarked_cb, (void *)(&context));
}

void kad_contactheap_done_cb(const kad_contact_t *c, void *user)
{
    bool *empty = (bool *)(user);
    *empty = false;
}

bool kad_contactheap_done(const kad_contactheap_t *s)
{
    bool empty = true;
    kad_contactheap_unmarked(s, kad_contactheap_done_cb, (void *)(&empty));
    return empty;
}

void kad_contactheap_remove(kad_contactheap_t *s, const kad_id_t *id)
{
    int id_i = -1;
    for (int i = 0; i < s->heap_size; i++)
    {
        if (kad_uint256_cmp(&s->heap_contacts[i].id, id) == 0)
        {
            id_i = i;
            break;
        }
    }

    if (id_i >= 0)
    {
        s->heap_size--;
        memmove(&s->heap_contacts[id_i], &s->heap_contacts[id_i + 1], (s->heap_size - id_i) * sizeof(kad_contact_t));
        memmove(&s->heap_distances[id_i], &s->heap_distances[id_i + 1], (s->heap_size - id_i) * sizeof(kad_uint256_t));
    }
}
