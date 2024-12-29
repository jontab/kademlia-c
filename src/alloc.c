#include "alloc.h"
#include "log.h"
#include <stdlib.h>

void *kad_alloc(size_t count, size_t size)
{
    void *data = calloc(count, size);
    kad_check(data, "kad_alloc failed");
    return data;
}

void *kad_realloc(void *data, size_t size)
{
    data = realloc(data, size);
    kad_check(data, "kad_realloc failed");
    return data;
}

void kad_check(void *data, const char *message)
{
    if (!data)
    {
        kad_fatal("%s\n", message);
        abort();
    }
}
