#ifndef KADEMLIA_LOGGING_H
#define KADEMLIA_LOGGING_H

#include <stdio.h>

#define DEBUG(FMT, ...) kad_logging_printf(KAD_LL_DEBUG, __FILE__ ":%d: " FMT, __LINE__, ##__VA_ARGS__)
#define INFO(FMT, ...)  kad_logging_printf(KAD_LL_INFO, __FILE__ ":%d: " FMT, __LINE__, ##__VA_ARGS__)
#define WARN(FMT, ...)  kad_logging_printf(KAD_LL_WARN, __FILE__ ":%d: " FMT, __LINE__, ##__VA_ARGS__)
#define ERROR(FMT, ...) kad_logging_printf(KAD_LL_ERROR, __FILE__ ":%d: " FMT, __LINE__, ##__VA_ARGS__)
#define FATAL(FMT, ...) kad_logging_printf(KAD_LL_FATAL, __FILE__ ":%d: " FMT, __LINE__, ##__VA_ARGS__)

/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

enum kad_logging_level_e
{
    KAD_LL_DEBUG,
    KAD_LL_INFO,
    KAD_LL_WARN,
    KAD_LL_ERROR,
    KAD_LL_FATAL,
};

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef enum kad_logging_level_e kad_logging_level_t;

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void        kad_logging_set_file(FILE *out);
void        kad_logging_set_level(kad_logging_level_t level);
void        kad_logging_printf(kad_logging_level_t level, const char *format, ...);
const char *kad_logging_level_string(kad_logging_level_t level);

#endif // KADEMLIA_LOGGING_H
