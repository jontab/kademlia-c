#ifndef KADEMLIA_RPC_H
#define KADEMLIA_RPC_H

#include "contact.h"
#include "uint256.h"

enum kad_request_type_e
{
    KAD_PING,
    KAD_STORE,
    KAD_FIND_NODE,
    KAD_FIND_VALUE,
};

typedef enum kad_request_type_e         kad_request_type_t;
typedef struct kad_ping_request_s       kad_ping_request_t;
typedef struct kad_store_request_s      kad_store_request_t;
typedef struct kad_find_node_request_s  kad_find_node_request_t;
typedef struct kad_find_value_request_s kad_find_value_request_t;
typedef struct kad_ping_result_s        kad_ping_result_t;
typedef struct kad_store_result_s       kad_store_result_t;
typedef struct kad_find_node_result_s   kad_find_node_result_t;
typedef struct kad_find_value_result_s  kad_find_value_result_t;
typedef struct kad_request_s            kad_request_t;
typedef struct kad_result_s             kad_result_t;

struct kad_ping_request_s
{
    kad_id_t id;
};

struct kad_store_request_s
{
    kad_id_t id;
    char    *key;
    char    *value;
};

struct kad_find_node_request_s
{
    kad_id_t id;
    kad_id_t target_id;
};

struct kad_find_value_request_s
{
    kad_id_t id;
    char    *key;
};

struct kad_ping_result_s
{
    kad_id_t id;
};

struct kad_store_result_s
{
};

struct kad_find_node_result_s
{
    kad_contact_t *contacts;
    int            size;
};

struct kad_find_value_result_s
{
    char          *value;
    kad_contact_t *contacts;
    int            size;
};

struct kad_request_s
{
    kad_request_type_t type;
    union {
        kad_ping_request_t       ping;
        kad_store_request_t      store;
        kad_find_node_request_t  find_node;
        kad_find_value_request_t find_value;
    } d;
};

struct kad_result_s
{
    kad_request_type_t type;
    union {
        kad_ping_result_t       ping;
        kad_store_result_t      store;
        kad_find_node_result_t  find_node;
        kad_find_value_result_t find_value;
    } d;
};

char *create_ping_request(kad_id_t *id, int *out_request_id);
char *create_store_request(kad_id_t *id, const char *key, const char *value, int *out_request_id);
char *create_find_node_request(kad_id_t *id, kad_id_t *target, int *out_request_id);
char *create_find_value_request(kad_id_t *id, const char *key, int *out_request_id);
char *create_ping_response(kad_id_t *server_id, int request_id);
char *create_store_response(int request_id);
char *create_find_node_response(kad_contact_t *contacts, int size, int request_id);
char *create_find_value_response(const char *value, kad_contact_t *contacts, int size, int request_id);

//
// Parsing
//

void *kad_payload_parse(const char *buf, int size);
bool  kad_payload_is_request(void *data);
bool  kad_payload_request_id(void *data, int *request_id);
bool  kad_payload_parse_request(void *data, kad_request_t *out);
bool  kad_payload_parse_result(void *data, int type, kad_result_t *out);
void  kad_request_fini(kad_request_t *s, void *data);
void  kad_result_fini(kad_result_t *s, void *data);

#endif // KADEMLIA_RPC_H
