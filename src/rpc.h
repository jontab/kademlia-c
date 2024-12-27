#ifndef KADEMLIA_RPC_H
#define KADEMLIA_RPC_H

#include "cJSON.h"
#include "uint256.h"

cJSON *create_ping_request(kad_id_t *id);
cJSON *create_find_node_request(kad_id_t *id, kad_id_t *target);
cJSON *create_store_request(kad_id_t *id, const char *key, const char *value);
cJSON *create_find_value_request(kad_id_t *id, const char *key);

#endif // KADEMLIA_RPC_H
