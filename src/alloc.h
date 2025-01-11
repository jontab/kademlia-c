#ifndef KAD_ALLOC_H
#define KAD_ALLOC_H

#include <stddef.h>

void *kad_alloc(size_t count, size_t size);
void *kad_realloc(void *data, size_t size);
void  kad_check(void *data, const char *message);
char *kad_strdup(const char *data);

#endif // KAD_ALLOC_H
