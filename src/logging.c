#include "logging.h"
#include <stdarg.h>
#include <time.h>

static FILE               *logging_file;
static kad_logging_level_t logging_level = KAD_LL_INFO;

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_logging_set_file(FILE *out)
{
    logging_file = out;
}

void kad_logging_set_level(kad_logging_level_t level)
{
    logging_level = level;
}

void kad_logging_printf(kad_logging_level_t level, const char *format, ...)
{
    if (!logging_file || (level < logging_level))
    {
        return;
    }

    time_t     now = time(NULL);
    struct tm *tm = localtime(&now);
    fprintf(logging_file, "[%02d-%02d-%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
    fprintf(logging_file, "[%-8s] ", kad_logging_level_string(level));

    va_list args;
    va_start(args, format);
    vfprintf(logging_file, format, args);
    va_end(args);

    fprintf(logging_file, "\n");
    fflush(logging_file);
}

const char *kad_logging_level_string(kad_logging_level_t level)
{
    switch (level)
    {
    case KAD_LL_DEBUG:
        return "debug";
    case KAD_LL_INFO:
        return "info";
    case KAD_LL_WARN:
        return "warn";
    case KAD_LL_ERROR:
        return "error";
    case KAD_LL_FATAL:
        return "fatal";
    default:
        return "unknown";
    }
}
