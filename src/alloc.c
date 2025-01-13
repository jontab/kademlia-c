#include "alloc.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

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
        FATAL("%s\n", message);
        abort();
    }
}

char *kad_strdup(const char *data)
{
    char *copy = strdup(data);
    kad_check(copy, "kad_strdup failed");
    return copy;
}
