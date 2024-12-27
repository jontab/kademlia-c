#ifndef KADEMLIA_PROTOCOL_H
#define KADEMLIA_PROTOCOL_H

#include "uint256.h"

#include <stdbool.h>

typedef struct kad_protocol_s        kad_protocol_t;
typedef struct kad_ping_args_s       kad_ping_args_t;
typedef struct kad_store_args_s      kad_store_args_t;
typedef struct kad_find_node_args_s  kad_find_node_args_t;
typedef struct kad_find_value_args_s kad_find_value_args_t;

typedef void (*kad_ping_cb_t)(bool ok, void *user);
typedef void (*kad_store_cb_t)(bool ok, void *user);
typedef void (*kad_find_node_cb_t)(bool ok, void *user);
typedef void (*kad_find_value_cb_t)(bool ok, void *user);

struct kad_ping_args_s
{
    kad_protocol_t *self;
    kad_id_t       *id;
    const char     *host;
    int             port;
    kad_ping_cb_t   callback;
    void           *user;
};

struct kad_store_args_s
{
};

struct kad_find_node_args_s
{
};

struct kad_find_value_args_s
{
};

struct kad_protocol_s
{
    void (*ping)(const kad_ping_args_t *args);
    void (*store)(const kad_store_args_t *args);
    void (*find_node)(const kad_find_node_args_t *args);
    void (*find_value)(const kad_find_value_args_t *args);
    void (*free)(kad_protocol_t *self);
};

#endif // KADEMLIA_PROTOCOL_H
