#ifndef KADEMLIA_RPC_H
#define KADEMLIA_RPC_H

#include "uint256.h"

char *create_ping_request(kad_id_t *id, int *out_request_id);
char *create_store_request(kad_id_t *id, const char *key, const char *value, int *out_request_id);
char *create_find_node_request(kad_id_t *id, kad_id_t *target, int *out_request_id);
char *create_find_value_request(kad_id_t *id, const char *key, int *out_request_id);
char *create_ping_response(kad_id_t *server_id, int request_id);

#endif // KADEMLIA_RPC_H
