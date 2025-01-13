#ifndef KAD_CLIENT_H
#define KAD_CLIENT_H

#include "protocol.h"
#include "storage.h"
#include "table.h"

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct kad_client_s kad_client_t;

typedef void (*bootstrap_then_t)(void *user);
typedef void (*lookup_then_t)(const char *value, void *user);
typedef void (*insert_then_t)(void *user);

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct kad_client_s
{
    const char        *host; // Args.
    int                port; // Args.
    kad_storage_t      storage;
    kad_id_t           id;
    int                capacity;
    kad_table_t        table;
    uv_loop_t         *loop;
    kad_uv_protocol_t *protocol;
};

/******************************************************************************/
/* Methods                                                                    */
/******************************************************************************/

void kad_client_init(kad_client_t *self, uv_loop_t *loop, const char *host, int port);
void kad_client_fini(kad_client_t *self);
void kad_client_start(kad_client_t *self);
void kad_client_bootstrap(kad_client_t *self, const char **hosts, int *ports, int size, bootstrap_then_t then,
                          void *user);
void kad_client_lookup(kad_client_t *self, const char *key, lookup_then_t then, void *user);
void kad_client_insert(kad_client_t *self, const char *key, const char *value, insert_then_t then, void *user);

#endif // KAD_CLIENT_H
