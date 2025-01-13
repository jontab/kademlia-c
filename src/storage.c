#include "storage.h"
#include "alloc.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

static kad_storagenode_t *kad_storagenode_new(const char *key, const char *value);
static void               kad_storagenode_free(kad_storagenode_t *self);

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_storage_init(kad_storage_t *self)
{
    memset(self, 0, sizeof(*self));
}

void kad_storage_fini(kad_storage_t *self)
{
    kad_storagenode_t *curr = self->head;
    while (curr)
    {
        kad_storagenode_t *next = curr->next;
        kad_storagenode_free(curr);
        curr = next;
    }

    memset(self, 0, sizeof(*self));
}

const char *kad_storage_lookup(kad_storage_t *self, const char *key)
{
    kad_storagenode_t *curr = self->head;
    const char        *result = NULL;

    while (curr)
    {
        if (strcmp(curr->key, key) == 0)
        {
            result = curr->value;
            break;
        }

        curr = curr->next;
    }

    INFO("storage[%s] is %s", key, result);
    return result;
}

void kad_storage_put(kad_storage_t *self, const char *key, const char *value)
{
    INFO("storage[%s] = %s", key, value);
    kad_storagenode_t *node = kad_storagenode_new(key, value);
    if (self->tail)
    {
        self->tail = (self->tail->next = node);
    }
    else
    {
        self->head = self->tail = node;
    }
}

void kad_storage_erase(kad_storage_t *self, const char *key)
{
}

/******************************************************************************/
/* Statics                                                                    */
/******************************************************************************/

kad_storagenode_t *kad_storagenode_new(const char *key, const char *value)
{
    kad_storagenode_t *self = kad_alloc(1, sizeof(kad_storagenode_t));
    clock_gettime(CLOCK_MONOTONIC_RAW, &self->created_at);
    self->key = kad_strdup(key);
    self->value = kad_strdup(value);
    self->next = NULL;
    return self;
}

void kad_storagenode_free(kad_storagenode_t *self)
{
    free(self->key);
    free(self->value);
    free(self);
}
