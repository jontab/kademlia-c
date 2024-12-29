#include "ordereddict.h"
#include "alloc.h"
#include "uint256.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void                   kad_ordereddictnode_free(kad_ordereddictnode_t *n);
static kad_ordereddictnode_t *kad_ordereddictnode_new(const kad_contact_t *c, kad_ordereddictnode_t *next);

//
// Public
//

void kad_ordereddict_init(kad_ordereddict_t *s)
{
    memset(s, 0, sizeof(*s));
}

void kad_ordereddict_fini(kad_ordereddict_t *s)
{
    if (s->head)
    {
        while (s->head)
        {
            kad_ordereddictnode_t *next = s->head->next;
            kad_ordereddictnode_free(s->head);
            s->head = next;
        }

        memset(s, 0, sizeof(*s));
    }
}

kad_ordereddict_t *kad_ordereddict_new(void)
{
    kad_ordereddict_t *s = kad_alloc(1, sizeof(kad_ordereddict_t));
    kad_ordereddict_init(s);
    return s;
}

void kad_ordereddict_free(kad_ordereddict_t *s)
{
    kad_ordereddict_fini(s);
    free(s);
}

/**
 * @param c Contact. Copies.
 */
void kad_ordereddict_insert(kad_ordereddict_t *s, const kad_contact_t *c)
{
    if (!s->head)
    {
        s->head = kad_ordereddictnode_new(c, NULL);
        s->size++;
    }
    else if (kad_uint256_cmp(&c->id, &s->head->c.id) == 0)
    {
        s->head->c = *c;
    }
    else
    {
        kad_ordereddictnode_t *prev = s->head;
        kad_ordereddictnode_t *curr = s->head->next;
        while (curr && kad_uint256_cmp(&c->id, &curr->c.id) != 0)
        {
            prev = curr;
            curr = curr->next;
        }

        if (curr && kad_uint256_cmp(&c->id, &curr->c.id) == 0)
        {
            curr->c = *c;
        }
        else
        {
            assert((curr == NULL) && "Expected end of list");
            prev->next = kad_ordereddictnode_new(c, NULL);
            s->size++;
        }
    }
}

bool kad_ordereddict_pop(kad_ordereddict_t *s, const kad_id_t *id, kad_contact_t *out)
{
    if (!s->head)
    {
        if (out)
        {
            memset(out, 0, sizeof(*out));
        }

        return false;
    }
    else if (kad_uint256_cmp(id, &s->head->c.id) == 0)
    {
        return kad_ordereddict_popfront(s, out);
    }
    else
    {
        kad_ordereddictnode_t *prev = s->head;
        kad_ordereddictnode_t *curr = s->head->next;
        while (curr && kad_uint256_cmp(id, &curr->c.id) != 0)
        {
            prev = curr;
            curr = curr->next;
        }

        if (curr && kad_uint256_cmp(id, &curr->c.id) == 0)
        {
            if (out)
            {
                *out = curr->c;
            }

            prev->next = curr->next;
            s->size--;
            kad_ordereddictnode_free(curr);
            return true;
        }
        else
        {
            if (out)
            {
                memset(out, 0, sizeof(*out));
            }

            return false;
        }
    }
}

bool kad_ordereddict_popfront(kad_ordereddict_t *s, kad_contact_t *out)
{
    if (!s->head)
    {
        if (out)
        {
            memset(out, 0, sizeof(*out));
        }

        return false;
    }
    else if (s->size == 1)
    {
        // This is the only node in the list.
        if (out)
        {
            *out = s->head->c;
        }

        kad_ordereddictnode_free(s->head);
        kad_ordereddict_init(s);
        return true;
    }
    else
    {
        // There are other nodes in the list.
        if (out)
        {
            *out = s->head->c;
        }

        kad_ordereddictnode_t *del = s->head;
        s->head = s->head->next;
        s->size--;
        kad_ordereddictnode_free(del);
        return true;
    }
}

bool kad_ordereddict_popback(kad_ordereddict_t *s, kad_contact_t *out)
{
    if (!s->head)
    {
        if (out)
        {
            memset(out, 0, sizeof(*out));
        }

        return false;
    }
    else if (s->size == 1)
    {
        return kad_ordereddict_popfront(s, out);
    }
    else
    {
        kad_ordereddictnode_t *prev = s->head;
        kad_ordereddictnode_t *curr = prev->next;
        while (curr->next)
        {
            prev = curr;
            curr = curr->next;
        }

        if (out)
        {
            *out = curr->c;
        }

        prev->next = curr->next;
        s->size--;
        kad_ordereddictnode_free(curr);
        return true;
    }
}

//
// Static
//

void kad_ordereddictnode_free(kad_ordereddictnode_t *n)
{
    free(n);
}

kad_ordereddictnode_t *kad_ordereddictnode_new(const kad_contact_t *c, kad_ordereddictnode_t *next)
{
    kad_ordereddictnode_t *n = kad_alloc(1, sizeof(kad_ordereddictnode_t));
    n->c = *c;
    n->next = next;
    return n;
}

int kad_ordereddict_iter(const kad_ordereddict_t *s, int (*cb)(const kad_contact_t *c, void *data), void *data)
{
    kad_ordereddictnode_t *curr = s->head;
    while (curr)
    {
        int ret = cb(&curr->c, data);
        if (ret == ITER_STOP)
        {
            return ret;
        }

        curr = curr->next;
    }

    return ITER_CONT;
}

bool kad_ordereddict_contains(const kad_ordereddict_t *s, const kad_id_t *id)
{
    kad_ordereddictnode_t *curr = s->head;
    while (curr)
    {
        if (kad_uint256_cmp(&curr->c.id, id) == 0)
        {
            return true;
        }

        curr = curr->next;
    }

    return false;
}
