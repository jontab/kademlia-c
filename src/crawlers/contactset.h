#ifndef KAD_CONTACTSET_H
#define KAD_CONTACTSET_H

#include "../contact.h"
#include <stdbool.h>

//
// Enums
//

enum kad_contactsetcellstate_e
{
    KAD_CONTACTSETCELLSTATE_EMPTY,
    KAD_CONTACTSETCELLSTATE_OCCUPIED,
    KAD_CONTACTSETCELLSTATE_DELETED,
};

//
// Typedefs
//

typedef struct kad_contactsetcell_s    kad_contactsetcell_t;
typedef struct kad_contactset_s        kad_contactset_t;
typedef enum kad_contactsetcellstate_e kad_contactsetcellstate_t;

typedef bool (*kad_contactsetiter_t)(const kad_contact_t *c, void *user);

//
// Structs
//

struct kad_contactsetcell_s
{
    kad_contact_t             contact;
    kad_contactsetcellstate_t st;
};

struct kad_contactset_s
{
    kad_contactsetcell_t *cells;
    int                   capacity;
    int                   size;
};

//
// Methods
//

void kad_contactset_init(kad_contactset_t *s);
void kad_contactset_fini(kad_contactset_t *s);
void kad_contactset_add(kad_contactset_t *s, const kad_contact_t *c);
bool kad_contactset_contains(const kad_contactset_t *s, const kad_id_t *id);
void kad_contactset_iter(const kad_contactset_t *s, kad_contactsetiter_t iter, void *user);
bool kad_contactset_pop(kad_contactset_t *s, const kad_id_t *id, kad_contact_t *out);

#endif // KAD_CONTACTSET_H
