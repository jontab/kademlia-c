#include "client.h"
#include "alloc.h"
#include "crawlers/nodecrawler.h"
#include "crawlers/valuecrawler.h"
#include "logging.h"
#include "logging.h"

#define DEFAULT_ALPHA 8

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct bootstrap_context_s bootstrap_context_t;
typedef struct insert_context_s    insert_context_t;
typedef struct lookup_context_s    lookup_context_t;

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct bootstrap_context_s
{
    kad_client_t      *self;
    kad_contact_t     *contacts;      // Grows.
    int               *contacts_size; // Grows.
    int               *bootstrap_result_count;
    int                bootstrap_size;
    bootstrap_then_t   then;
    void              *then_data;
    char               contact_host[BUFSIZ];
    int                contact_port;
    kad_nodecrawler_t *crawler;
};

struct insert_context_s
{
    char               key[BUFSIZ];
    char               value[BUFSIZ];
    kad_client_t      *self;
    insert_then_t      then;
    void              *then_data;
    int                total_store_count;
    int                store_count;
    kad_nodecrawler_t *crawler;
};

struct lookup_context_s
{
    kad_valuecrawler_t *crawler;
    lookup_then_t       then;
    void               *then_data;
};

/******************************************************************************/
/* Decls                                                                      */
/******************************************************************************/

static void kad_client_schedule_refresh(kad_client_t *self);
static void kad_client_bootstrap_ping_callback(bool ok, void *result, void *user);
static void kad_client_bootstrap_process_results(bootstrap_context_t *context);
static void kad_client_bootstrap_crawler_callback(const kad_contact_t *contacts, int contacts_size, void *user);
static void kad_client_insert_callback(const kad_contact_t *contacts, int contacts_size, void *user);
static void kad_client_insert_callback_store(bool ok, void *result, void *user);
static void kad_client_lookup_callback(const char *value, void *user);
static int  insort_compare(kad_contact_t *left, kad_contact_t *right, void *user);

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_client_init(kad_client_t *self, uv_loop_t *loop, const char *host, int port)
{
    memset(self, 0, sizeof(*self));
    self->host = host;
    self->port = port;
    self->id = (kad_id_t){{rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand()}};
    self->capacity = 32;
    self->loop = loop;

    kad_storage_init(&self->storage);
    kad_table_init(&self->table, &self->id, self->capacity);

    self->protocol = kad_uv_protocol_new(self->loop, &self->table, &self->storage);
    kad_table_set_protocol(&self->table, (kad_protocol_t *)(self->protocol));
}

void kad_client_fini(kad_client_t *self)
{
    kad_uv_protocol_free(self->protocol);
    kad_table_fini(&self->table);
    kad_storage_fini(&self->storage);
    memset(self, 0, sizeof(*self));
}

void kad_client_start(kad_client_t *self)
{
    kad_uv_protocol_start(self->protocol, self->host, self->port);
    kad_client_schedule_refresh(self);
    INFO("Listening on %s:%d", self->host, self->port);
}

void kad_client_bootstrap(kad_client_t *self, const char **hosts, int *ports, int size, bootstrap_then_t then,
                          void *user)
{
    INFO("Bootstrapping: with %d addresses", size);
    if (size == 0)
    {
        then(user);
        return;
    }

    kad_contact_t *contacts = NULL;
    int           *contacts_size = kad_alloc(1, sizeof(int));
    int           *bootstrap_result_count = kad_alloc(1, sizeof(int));
    for (int contact_i = 0; contact_i < size; contact_i++)
    {
        const char *contact_host = hosts[contact_i];
        int         contact_port = ports[contact_i];
        INFO("Bootstrapping: pinging %s:%d", contact_host, contact_port);

        bootstrap_context_t *context = kad_alloc(1, sizeof(bootstrap_context_t));
        *context = (bootstrap_context_t){
            .self = self,
            .contacts = contacts,
            .contacts_size = contacts_size,
            .bootstrap_result_count = bootstrap_result_count,
            .bootstrap_size = size,
            .then = then,
            .then_data = user,
            .contact_host = {0},
            .contact_port = contact_port,
        };
        strncpy(context->contact_host, contact_host, sizeof(context->contact_host) - 1);

        self->protocol->base.ping(&(kad_ping_args_t){
            .self = &self->protocol->base,
            .id = &self->id,
            .host = contact_host,
            .port = contact_port,
            .callback = kad_client_bootstrap_ping_callback,
            .user = context,
        });
    }
}

void kad_client_lookup(kad_client_t *self, const char *key, lookup_then_t then, void *user)
{
    kad_contactlist_t contacts = {0};
    kad_table_find_closest(&self->table, &self->id, &self->id, &contacts);
    kad_valuecrawlerargs_t args = {
        .id = self->id,
        .key = key,
        .capacity = self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = contacts.data,
        .contacts_size = contacts.size,
        .proto = (kad_protocol_t *)(self->protocol),
    };
    lookup_context_t *context = kad_alloc(1, sizeof(lookup_context_t));
    *context = (lookup_context_t){
        .then = then,
        .then_data = user,
        .crawler = kad_alloc(1, sizeof(kad_valuecrawler_t)),
    };
    kad_valuecrawler_init(context->crawler, &args);
    kad_valuecrawler_find(context->crawler, kad_client_lookup_callback, context);
    kad_contactlist_fini(&contacts);
}

void kad_client_insert(kad_client_t *self, const char *key, const char *value, insert_then_t then, void *user)
{
    kad_contactlist_t contacts = {0};
    kad_table_find_closest(&self->table, &self->id, &self->id, &contacts);
    kad_nodecrawlerargs_t args = {
        .id = self->id,
        .target = {0},
        .capacity = self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = contacts.data,
        .contacts_size = contacts.size,
        .proto = (kad_protocol_t *)(self->protocol),
    };
    kad_uint256_from_key(key, &args.target);
    insert_context_t *context = kad_alloc(1, sizeof(insert_context_t));
    *context = (insert_context_t){
        .key = {0},
        .value = {0},
        .self = self,
        .then = then,
        .then_data = user,
        .crawler = kad_alloc(1, sizeof(kad_nodecrawler_t)),
    };
    strncpy(context->key, key, sizeof(context->key) - 1);
    strncpy(context->value, value, sizeof(context->value) - 1);
    kad_nodecrawler_init(context->crawler, &args);
    kad_nodecrawler_find(context->crawler, kad_client_insert_callback, context);
    kad_contactlist_fini(&contacts);
}

/******************************************************************************/
/* Statics                                                                    */
/******************************************************************************/

void kad_client_schedule_refresh(kad_client_t *self)
{
}

void kad_client_bootstrap_ping_callback(bool ok, void *result, void *user)
{
    bootstrap_context_t *context = (bootstrap_context_t *)(user);
    kad_result_t        *result_obj = (kad_result_t *)(result_obj);
    INFO("Bootstrapping: got ping back from %s:%d - ok: %d", context->contact_host, context->contact_port, ok);
    if (ok)
    {
        (*context->contacts_size)++;
        context->contacts = kad_realloc(context->contacts, (*context->contacts_size) * sizeof(kad_contact_t));

        // Copy the result into the array.
        kad_contact_t *dest_contact = &context->contacts[*context->contacts_size - 1];
        dest_contact->id = result_obj->d.ping.id;
        dest_contact->port = context->contact_port;
        strncpy(dest_contact->host, context->contact_host, sizeof(dest_contact->host) - 1);
    }

    (*context->bootstrap_result_count)++;
    if (*context->bootstrap_result_count == context->bootstrap_size)
    {
        // We've received results from all of the nodes.
        INFO("Bootstrapping: finished pinging");
        kad_client_bootstrap_process_results(context);
        if (context->contacts)
        {
            free(context->contacts);
        }

        free(context->contacts_size);
        free(context->bootstrap_result_count);

        context->contacts = NULL;
        context->contacts_size = NULL;
        context->bootstrap_result_count = NULL;
    }
}

void kad_client_bootstrap_process_results(bootstrap_context_t *context)
{
    kad_table_t *table = &context->self->table;
    for (int i = 0; i < *context->contacts_size; i++)
    {
        kad_contact_t *contact = &context->contacts[i];
        kad_table_add_contact(table, contact);
    }

    // Add neighbors to table.
    INFO("Bootstrapping: searching for neighbors");
    kad_nodecrawlerargs_t args = {
        .id = table->id,
        .proto = (kad_protocol_t *)(context->self->protocol),
        .target = table->id,
        .capacity = context->self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = context->contacts,
        .contacts_size = *context->contacts_size,
    };
    context->crawler = kad_alloc(1, sizeof(kad_nodecrawler_t));
    kad_nodecrawler_init(context->crawler, &args);
    kad_nodecrawler_find(context->crawler, kad_client_bootstrap_crawler_callback, context);
}

void kad_client_bootstrap_crawler_callback(const kad_contact_t *contacts, int contacts_size, void *user)
{
    bootstrap_context_t *context = (bootstrap_context_t *)(user);
    kad_table_t         *table = &context->self->table;
    INFO("Found %d neighbors", contacts_size);

    // Add neighbors to table.
    for (int i = 0; i < contacts_size; i++)
    {
        const kad_contact_t *contact = &contacts[i];
        kad_table_add_contact(table, contact);
    }

    kad_nodecrawler_fini(context->crawler);

    // Bootstrapping is done!
    context->then(context->then_data);
}

void kad_client_insert_callback(const kad_contact_t *contacts, int contacts_size, void *user)
{
    insert_context_t *context = (insert_context_t *)(user);

    kad_contactlist_t insert_into = {0};
    int               insert_size = MIN(contacts_size + 1, context->self->capacity);
    kad_contactlist_clone(&insert_into, contacts, contacts_size);
    kad_contactlist_insort(&insert_into, (kad_contact_t){.id = context->self->id}, insort_compare, context);

    // Determine the number of outbound calls we are expecting.
    context->total_store_count = 0; // # of outbound calls (excl. ourselves).
    context->store_count = 0;
    for (int i = 0; i < insert_size; i++)
    {
        kad_contact_t *contact = &insert_into.data[i];
        if (kad_uint256_cmp(&contact->id, &context->self->id) == 0)
        {
        }
        else
        {
            context->total_store_count++;
        }
    }

    for (int i = 0; i < insert_size; i++)
    {
        kad_contact_t *contact = &insert_into.data[i];
        if (kad_uint256_cmp(&contact->id, &context->self->id) == 0)
        {
            DEBUG("Inserting key into ourselves");
            kad_storage_put(&context->self->storage, context->key, context->value);
        }
        else
        {
            DEBUG("Inserting key into another node");
            kad_client_t   *self = context->self;
            kad_protocol_t *protocol = (kad_protocol_t *)(self->protocol);

            const char *contact_host = insert_into.data[i].host;
            int         contact_port = insert_into.data[i].port;

            protocol->store(&(kad_store_args_t){
                .self = protocol,
                .id = &self->id,
                .key = context->key,
                .value = context->value,
                .host = contact_host,
                .port = contact_port,
                .callback = kad_client_insert_callback_store,
                .user = context,
            });
        }
    }

    kad_contactlist_fini(&insert_into);
}

void kad_client_insert_callback_store(bool ok, void *result, void *user)
{
    insert_context_t *context = (insert_context_t *)(user);
    if ((++context->store_count) == context->total_store_count)
    {
        insert_then_t then = context->then;
        void         *then_data = context->then_data;

        kad_nodecrawler_fini(context->crawler);
        free(context);

        then(then_data);
    }
}

void kad_client_lookup_callback(const char *value, void *user)
{
    lookup_context_t *context = (lookup_context_t *)(user);
    kad_valuecrawler_fini(context->crawler);
    context->then(value, context->then_data);
    free(context);
}

int insort_compare(kad_contact_t *left, kad_contact_t *right, void *user)
{
    insert_context_t *context = (insert_context_t *)(user);
    kad_uint256_t     ldist;
    kad_uint256_t     rdist;
    kad_uint256_xor(&left->id, &context->self->id, &ldist);
    kad_uint256_xor(&left->id, &context->self->id, &rdist);
    return kad_uint256_cmp(&ldist, &rdist);
}
