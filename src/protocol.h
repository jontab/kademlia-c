#ifndef KADEMLIA_PROTOCOL_H
#define KADEMLIA_PROTOCOL_H

#include "ds/list.h"
#include "rpc.h"
#include "storage.h"
#include "table.h"
#include "uint256.h"

#include <stdbool.h>
#include <uv.h>

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct kad_promise_s         kad_promise_t;
typedef struct kad_protocol_s        kad_protocol_t;
typedef struct kad_ping_args_s       kad_ping_args_t;
typedef struct kad_store_args_s      kad_store_args_t;
typedef struct kad_find_node_args_s  kad_find_node_args_t;
typedef struct kad_find_value_args_s kad_find_value_args_t;
typedef struct kad_uv_protocol_s     kad_uv_protocol_t;

typedef void (*kad_resolve_t)(bool ok, void *result, void *user);

KAD_GENERATE_LIST_HEADER(kad_promise_list, struct kad_promise_s *)

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct kad_promise_s
{
    int           id;
    kad_resolve_t resolve;
    void         *user;
    int           type;
};

struct kad_ping_args_s
{
    kad_protocol_t *self;
    kad_id_t       *id;
    const char     *host;
    int             port;
    kad_resolve_t   callback;
    void           *user;
};

struct kad_store_args_s
{
    kad_protocol_t *self;
    kad_id_t       *id;
    const char     *key;
    const char     *value;
    const char     *host;
    int             port;
    kad_resolve_t   callback;
    void           *user;
};

struct kad_find_node_args_s
{
    kad_protocol_t *self;
    kad_id_t       *id;
    kad_id_t       *target_id;
    const char     *host;
    int             port;
    kad_resolve_t   callback;
    void           *user;
};

struct kad_find_value_args_s
{
    kad_protocol_t *self;
    kad_id_t       *id;
    const char     *key;
    const char     *host;
    int             port;
    kad_resolve_t   callback;
    void           *user;
};

struct kad_protocol_s
{
    void (*ping)(const kad_ping_args_t *args);
    void (*store)(const kad_store_args_t *args);
    void (*find_node)(const kad_find_node_args_t *args);
    void (*find_value)(const kad_find_value_args_t *args);
};

struct kad_uv_protocol_s
{
    kad_protocol_t     base;
    kad_promise_list_t promises;
    uv_loop_t         *loop;
    uv_udp_t           socket;
    kad_table_t       *table;
    kad_storage_t     *storage;
};

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

kad_uv_protocol_t *kad_uv_protocol_new(uv_loop_t *loop, kad_table_t *table, kad_storage_t *storage);
void               kad_uv_protocol_free(kad_uv_protocol_t *self);
void               kad_uv_protocol_start(kad_uv_protocol_t *self, const char *host, int port);

#endif // KADEMLIA_PROTOCOL_H
