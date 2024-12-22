#ifndef KADEMLIA_LOG_H
#define KADEMLIA_LOG_H

#include <stdio.h>

#ifndef KADEMLIA_NDEBUG
#define kad_debug(...) kad_printf("[debug] " __VA_ARGS__)
#define kad_info(...) kad_printf("[info] " __VA_ARGS__)
#define kad_warn(...) kad_printf("[warn] " __VA_ARGS__)
#define kad_error(...) kad_printf("[error] " __VA_ARGS__)
#define kad_fatal(...) kad_printf("[fatal] " __VA_ARGS__)
#else
#define kad_debug(...)
#define kad_info(...)
#define kad_warn(...)
#define kad_error(...)
#define kad_fatal(...)
#endif // KADEMLIA_DEBUG

void kad_printf(const char *format, ...);

#endif // KADEMLIA_LOG_H
