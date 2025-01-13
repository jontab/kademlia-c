#ifndef KADEMLIA_LIST_H
#define KADEMLIA_LIST_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define LIST_DEFAULT_FREE(X)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        (void)(X);                                                                                                     \
    } while (0)

#define KAD_GENERATE_LIST_HEADER(NAME, TYPE)                                                                           \
    typedef struct NAME##_s NAME##_t;                                                                                  \
                                                                                                                       \
    struct NAME##_s                                                                                                    \
    {                                                                                                                  \
        TYPE *data;                                                                                                    \
        int   size;                                                                                                    \
        int   capacity;                                                                                                \
    };                                                                                                                 \
                                                                                                                       \
    void NAME##_fini(NAME##_t *l);                                                                                     \
    void NAME##_reserve(NAME##_t *l, int size);                                                                        \
    void NAME##_insert(NAME##_t *l, TYPE data, int at);                                                                \
    void NAME##_append(NAME##_t *l, TYPE data);                                                                        \
    void NAME##_remove(NAME##_t *l, int at);

#define KAD_GENERATE_LIST_SOURCE(NAME, TYPE, FREE)                                                                     \
    void NAME##_fini(NAME##_t *l)                                                                                      \
    {                                                                                                                  \
        for (int i = 0; i < l->size; i++)                                                                              \
        {                                                                                                              \
            FREE(l->data[i]);                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        if (l->data)                                                                                                   \
        {                                                                                                              \
            free(l->data);                                                                                             \
        }                                                                                                              \
                                                                                                                       \
        memset(l, 0, sizeof(*l));                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    void NAME##_reserve(NAME##_t *l, int size)                                                                         \
    {                                                                                                                  \
        if (l->capacity < size)                                                                                        \
        {                                                                                                              \
            l->capacity = size + 11;                                                                                   \
            l->data = realloc(l->data, l->capacity * sizeof(TYPE));                                                    \
            assert(l->data && "Out of memory");                                                                        \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    void NAME##_insert(NAME##_t *l, TYPE data, int at)                                                                 \
    {                                                                                                                  \
        NAME##_reserve(l, l->size + 1);                                                                                \
        memmove(&l->data[at + 1], &l->data[at], (l->size++ - at) * sizeof(TYPE));                                      \
        l->data[at] = data;                                                                                            \
    }                                                                                                                  \
                                                                                                                       \
    void NAME##_append(NAME##_t *l, TYPE data)                                                                         \
    {                                                                                                                  \
        NAME##_insert(l, data, l->size);                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    void NAME##_remove(NAME##_t *l, int at)                                                                            \
    {                                                                                                                  \
        FREE(l->data[at]);                                                                                             \
        memmove(&l->data[at], &l->data[at + 1], (--l->size - at) * sizeof(TYPE));                                      \
    }

#endif // KADEMLIA_LIST_H
