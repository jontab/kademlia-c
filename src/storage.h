#ifndef KAD_STORAGE_H
#define KAD_STORAGE_H

#include <time.h>

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct kad_storagenode_s kad_storagenode_t;
typedef struct kad_storage_s     kad_storage_t;

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct kad_storagenode_s
{
    char              *key;
    char              *value;
    kad_storagenode_t *next;
    struct timespec    created_at;
};

struct kad_storage_s
{
    // TODO: Complex storage.
    kad_storagenode_t *head;
    kad_storagenode_t *tail;
};

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void        kad_storage_init(kad_storage_t *self);
void        kad_storage_fini(kad_storage_t *self);
const char *kad_storage_lookup(kad_storage_t *self, const char *key);
void        kad_storage_put(kad_storage_t *self, const char *key, const char *value);
void        kad_storage_erase(kad_storage_t *self, const char *key);

#endif // KAD_STORAGE_H
