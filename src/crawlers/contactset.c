#include "contactset.h"
#include "../alloc.h"
#include "../log.h"
#include <stdlib.h>
#include <string.h>

static void         kad_contactset_reserve(kad_contactset_t *s, int size);
static unsigned int kad_contactset_hash(const kad_id_t *id);
static bool         kad_contactset_reserve_cb(const kad_contact_t *c, void *user);

//
// Public
//

void kad_contactset_init(kad_contactset_t *s)
{
    memset(s, 0, sizeof(*s));
    kad_contactset_reserve(s, 11);
}

void kad_contactset_fini(kad_contactset_t *s)
{
    free(s->cells);
    memset(s, 0, sizeof(*s));
}

void kad_contactset_add(kad_contactset_t *s, const kad_contact_t *c)
{
    if (kad_contactset_contains(s, &c->id))
    {
        return;
    }

    kad_contactset_reserve(s, s->size + 1);
    int start = kad_contactset_hash(&c->id) % s->capacity;
    int i = start;
    do
    {
        if ((s->cells[i].st == KAD_CONTACTSETCELLSTATE_DELETED) || (s->cells[i].st == KAD_CONTACTSETCELLSTATE_EMPTY))
        {
            s->cells[i].st = KAD_CONTACTSETCELLSTATE_OCCUPIED;
            s->cells[i].contact = *c;
            s->size++;
            break;
        }

        i = (i + 1) % s->capacity;
    } while (i != start);
}

bool kad_contactset_contains(const kad_contactset_t *s, const kad_id_t *id)
{
    int start = kad_contactset_hash(id) % s->capacity;
    int i = start;
    do
    {
        if ((s->cells[i].st == KAD_CONTACTSETCELLSTATE_OCCUPIED) && !kad_uint256_cmp(&s->cells[i].contact.id, id))
        {
            return true;
        }
        else if (s->cells[i].st == KAD_CONTACTSETCELLSTATE_DELETED)
        {
            return false;
        }

        i = (i + 1) % s->capacity;
    } while (i != start);
    return false;
}

void kad_contactset_iter(const kad_contactset_t *s, kad_contactsetiter_t iter, void *user)
{
    for (int i = 0; i < s->capacity; i++)
    {
        if (s->cells[i].st == KAD_CONTACTSETCELLSTATE_OCCUPIED)
        {
            if (iter(&s->cells[i].contact, user) == false)
            {
                return;
            }
        }
    }
}

bool kad_contactset_pop(kad_contactset_t *s, const kad_id_t *id, kad_contact_t *out)
{
    int start = kad_contactset_hash(id) % s->capacity;
    int i = start;
    do
    {
        if ((s->cells[i].st == KAD_CONTACTSETCELLSTATE_OCCUPIED) && !kad_uint256_cmp(&s->cells[i].contact.id, id))
        {
            if (out)
            {
                *out = s->cells[i].contact;
            }

            s->cells[i].st = KAD_CONTACTSETCELLSTATE_DELETED;
            s->size--;
            return true;
        }
        else if (s->cells[i].st == KAD_CONTACTSETCELLSTATE_DELETED)
        {
            return false;
        }

        i = (i + 1) % s->capacity;
    } while (i != start);
    return false;
}

//
// Static
//

void kad_contactset_reserve(kad_contactset_t *s, int size)
{
    if (size > s->capacity / 2)
    {
        kad_contactset_t copy = *s;
        memset(s, 0, sizeof(*s));

        s->capacity = size * 5 + 11;
        s->cells = kad_alloc(s->capacity, sizeof(kad_contactsetcell_t));

        kad_contactset_iter(&copy, kad_contactset_reserve_cb, (void *)(s));
        kad_contactset_fini(&copy);
    }
}

unsigned int kad_contactset_hash(const kad_id_t *id)
{
    // https://stackoverflow.com/questions/7666509/hash-function-for-string
    unsigned int hash = 5381;
    for (int i = 0; i < sizeof(*id) / sizeof(id->d[0]); i++)
    {
        hash = ((hash << 5) + hash) + id->d[i];
    }

    return hash;
}

bool kad_contactset_reserve_cb(const kad_contact_t *c, void *user)
{
    kad_contactset_t *s = (kad_contactset_t *)(user);
    kad_contactset_add(s, c);
    return true;
}
