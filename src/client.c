#include "client.h"
#include "alloc.h"
#include "crawlers/nodecrawler.h"
#include "crawlers/valuecrawler.h"
#include "log.h"

#define DEFAULT_ALPHA 8

//
// Typedefs
//

typedef struct bootstrap_context_s bootstrap_context_t;
typedef struct insert_context_s    insert_context_t;

//
// Structs
//

struct bootstrap_context_s
{
    kad_client_t    *self;
    kad_contact_t   *contacts;      // Grows.
    int             *contacts_size; // Grows.
    int             *bootstrap_result_count;
    int              bootstrap_size;
    bootstrap_then_t then;
    void            *then_data;
    char             contact_host[BUFSIZ];
    int              contact_port;
};

struct insert_context_s
{
    char          key[BUFSIZ];
    char          value[BUFSIZ];
    kad_client_t *self;
    insert_then_t then;
    void         *then_data;
    int           total_store_count;
    int           store_count;
};

//
// Decls
//

static void kad_client_schedule_refresh(kad_client_t *self);
static void kad_client_bootstrap_ping_callback(bool ok, void *result, void *user);
static void kad_client_bootstrap_process_results(bootstrap_context_t *context);
static void kad_client_bootstrap_crawler_callback(const kad_contact_t *contacts, int contacts_size, void *user);
static void kad_client_insert_callback(const kad_contact_t *contacts, int contacts_size, void *user);
static void kad_client_insert_callback_store(bool ok, void *result, void *user);

//
// Public
//

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
    kad_info("listening on %s:%d\n", self->host, self->port);
}

void kad_client_bootstrap(kad_client_t *self, const char **hosts, int *ports, int size, void (*then)(void *user),
                          void *user)
{
    kad_info("bootstrapping with %d address\n", size);
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
        kad_info("sending a ping to %s:%d\n", contact_host, contact_port);

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

//
// Statics
//

void kad_client_schedule_refresh(kad_client_t *self)
{
    kad_info("scheduling refresh\n");
}

void kad_client_bootstrap_ping_callback(bool ok, void *result, void *user)
{
    bootstrap_context_t *context = (bootstrap_context_t *)(user);
    kad_result_t        *result_obj = (kad_result_t *)(result_obj);
    kad_info("got ping result from %s:%d - ok: %d\n", context->contact_host, context->contact_port, ok);

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
        kad_info("finished pinging bootstrap nodes\n");

        // We've received results from all of the nodes.
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

    // Add bootstraps to table.
    for (int i = 0; i < *context->contacts_size; i++)
    {
        kad_contact_t *contact = &context->contacts[i];
        kad_table_add_contact(table, contact);
    }

    // Add neighbors to table.
    kad_info("issuing search for neighbor nodes\n");

    kad_nodecrawlerargs_t args = {
        .id = table->id,
        .proto = (kad_protocol_t *)(context->self->protocol),
        .target = table->id,
        .capacity = context->self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = context->contacts,
        .contacts_size = *context->contacts_size,
    };
    kad_nodecrawler_t *crawler = kad_alloc(1, sizeof(kad_nodecrawler_t));
    kad_nodecrawler_init(crawler, &args);
    kad_nodecrawler_find(crawler, kad_client_bootstrap_crawler_callback, context);
}

void kad_client_bootstrap_crawler_callback(const kad_contact_t *contacts, int contacts_size, void *user)
{
    bootstrap_context_t *context = (bootstrap_context_t *)(user);
    kad_table_t         *table = &context->self->table;
    kad_info("adding %d neighbors to table\n", contacts_size);

    // Add neighbors to table.
    for (int i = 0; i < contacts_size; i++)
    {
        const kad_contact_t *contact = &contacts[i];
        kad_table_add_contact(table, contact);
    }

    // Bootstrapping is done!
    context->then(context->then_data);
    // TODO: Delete crawler.
}

void kad_client_lookup(kad_client_t *self, const char *key, lookup_then_t then, void *user)
{
    kad_contact_t *contacts = NULL;
    int            contacts_size = 0;
    kad_table_find_closest(&self->table, &self->id, &self->id, &contacts, &contacts_size);

    kad_valuecrawlerargs_t args = {
        .id = self->id,
        .key = key,
        .capacity = self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = contacts,
        .contacts_size = contacts_size,
        .proto = (kad_protocol_t *)(self->protocol),
    };
    kad_valuecrawler_t *crawler = kad_alloc(1, sizeof(kad_valuecrawler_t));
    kad_valuecrawler_init(crawler, &args);

    free(contacts);

    kad_valuecrawler_find(crawler, then, user);
    // TODO: Delete crawler.
}

void kad_client_insert(kad_client_t *self, const char *key, const char *value, insert_then_t then, void *user)
{
    kad_contact_t *contacts = NULL;
    int            contacts_size = 0;
    kad_table_find_closest(&self->table, &self->id, &self->id, &contacts, &contacts_size);

    kad_nodecrawlerargs_t args = {
        .id = self->id,
        .target = {0},
        .capacity = self->capacity,
        .alpha = DEFAULT_ALPHA,
        .contacts = contacts,
        .contacts_size = contacts_size,
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
    };
    strncpy(context->key, key, sizeof(context->key) - 1);
    strncpy(context->value, value, sizeof(context->value) - 1);

    kad_nodecrawler_t *crawler = kad_alloc(1, sizeof(kad_nodecrawler_t));
    kad_nodecrawler_init(crawler, &args);

    free(contacts);

    kad_nodecrawler_find(crawler, kad_client_insert_callback, context);

    // TODO: Clean crawler.
}

//
// Statics
//

void kad_client_insert_callback(const kad_contact_t *contacts, int contacts_size, void *user)
{
    insert_context_t *context = (insert_context_t *)(user);
    context->total_store_count = contacts_size;
    context->store_count = 0;

    for (int i = 0; i < contacts_size; i++)
    {
        kad_client_t   *self = context->self;
        kad_protocol_t *protocol = (kad_protocol_t *)(self->protocol);

        const char *contact_host = contacts[i].host;
        int         contact_port = contacts[i].port;

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

void kad_client_insert_callback_store(bool ok, void *result, void *user)
{
    insert_context_t *context = (insert_context_t *)(user);
    context->store_count++;

    // TODO: Should check `ok`?

    if (context->store_count == context->total_store_count)
    {
        insert_then_t then = context->then;
        void         *then_data = context->then_data;

        free(context);

        then(then_data);
    }
}
