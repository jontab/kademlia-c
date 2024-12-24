#ifndef KADEMLIA_PROTOCOL_H
#define KADEMLIA_PROTOCOL_H

#include <uv.h>

typedef struct kad_protocol_s kad_protocol_t;

void kad_protocol_init(kad_protocol_t *s, uv_loop_t *loop, const char *host, int port);
void kad_protocol_fini(kad_protocol_t *s);

#endif // KADEMLIA_PROTOCOL_H
