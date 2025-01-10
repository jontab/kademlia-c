#include "nodecrawler.h"
#include "../alloc.h"

//
// Typedefs
//

typedef struct n_findresults_s n_findresults_t;
typedef struct n_findcontext_s n_findcontext_t;
typedef struct n_nodecontext_s n_nodecontext_t;

//
// Structs
//

struct n_findresults_s
{
    kad_id_t     *ids;
    bool         *oks;
    kad_result_t *data;
    int           size;
};

struct n_findcontext_s
{
    kad_nodecrawler_t *s;
    n_findresults_t   *results;
    int                i;

    // Handle.
    kad_nodecrawlercb_t gotresult;
    void               *gotresultuser;
};

struct n_nodecontext_s
{
    kad_nodecrawler_t *s;
    n_findresults_t   *results;
    kad_id_t           id;

    // Handle.
    const char         *key;
    kad_nodecrawlercb_t gotresult;
    void               *gotresultuser;
};

//
// Decls
//

static void kad_nodecrawler_handle(kad_nodecrawler_t *s, n_findresults_t *results, kad_nodecrawlercb_t cb, void *user);
static void kad_nodecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user);
static void kad_nodecrawler_find_node_cb(bool ok, void *result, void *user);

//
// Public
//

void kad_nodecrawler_init(kad_nodecrawler_t *s, kad_nodecrawlerargs_t args)
{
    s->id = args.id;
    s->capacity = args.capacity;
    s->alpha = args.alpha;

    kad_id_t target;
    kad_uint256_from_key(args.key, &target);
    kad_contactheap_init(&s->nearest, &target, args.capacity);

    s->proto = args.proto;
    kad_contactset_init(&s->ignore);
}

void kad_nodecrawler_fini(kad_nodecrawler_t *s)
{
    kad_contactheap_fini(&s->nearest);
    kad_contactset_fini(&s->ignore);
    memset(s, 0, sizeof(*s));
}

void kad_nodecrawler_find(kad_nodecrawler_t *s, kad_nodecrawlercb_t cb, void *user)
{
    n_findcontext_t findcontext = {
        .s = s,
        .results = kad_alloc(1, sizeof(n_findresults_t)),

        // Handle.
        .gotresult = cb,
        .gotresultuser = user,
    };
    kad_contactheap_unmarked(&s->nearest, kad_nodecrawler_find_unmarked_cb, &findcontext);
}

//
// Statics
//

void kad_nodecrawler_handle(kad_nodecrawler_t *s, n_findresults_t *results, kad_nodecrawlercb_t cb, void *data)
{
    for (int i = 0; i < results->size; i++)
    {
        kad_contactheap_mark(&s->nearest, &results->ids[i]);
        if (!results->oks[i])
        {
            kad_contactheap_remove(&s->nearest, &results->ids[i]);
            kad_contactset_add(&s->ignore, &(kad_contact_t){.id = results->ids[i]});
            continue;
        }

        // Contacts.
        kad_contact_t *gotcontact = &results->data[i].d.find_node.contacts[i];
        if (!kad_contactset_contains(&s->ignore, &gotcontact->id))
        {
            kad_contactheap_push(&s->nearest, gotcontact);
        }
    }

    if (kad_contactheap_done(&s->nearest))
    {
        cb(NULL, data);
        return;
    }

    kad_nodecrawler_find(s, cb, data);
}

void kad_nodecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user)
{
    n_findcontext_t *fc = (n_findcontext_t *)(user);
    if (fc->i < fc->s->alpha)
    {
        n_nodecontext_t *vc = kad_alloc(1, sizeof(n_nodecontext_t));
        *vc = (n_nodecontext_t){
            .s = fc->s,
            .results = fc->results,
            .id = unmarked->id,
            .gotresult = fc->gotresult,
            .gotresultuser = fc->gotresultuser,
        };

        fc->s->proto->find_value(&(kad_find_value_args_t){
            .self = fc->s->proto,
            .id = &fc->s->id,
            .host = unmarked->host,
            .port = unmarked->port,
            .callback = kad_nodecrawler_find_node_cb,
            .user = vc,
        });
    }

    fc->i++;
}

void kad_nodecrawler_find_node_cb(bool ok, void *result, void *user)
{
    n_nodecontext_t *c = (n_nodecontext_t *)(user);

    c->results->size++;
    c->results->ids = kad_realloc(c->results->ids, c->results->size * sizeof(kad_id_t));
    c->results->data = kad_realloc(c->results->data, c->results->size * sizeof(kad_result_t));

    if (ok)
    {
        c->results->ids[c->results->size - 1] = c->id;
        c->results->oks[c->results->size - 1] = true;
        c->results->data[c->results->size - 1] = *((kad_result_t *)(result));
    }
    else
    {
        c->results->ids[c->results->size - 1] = c->id;
        c->results->oks[c->results->size - 1] = false;
        c->results->data[c->results->size - 1] = (kad_result_t){0};
    }

    if (c->results->size == c->s->alpha)
    {
        kad_nodecrawler_handle(c->s, c->results, c->gotresult, c->gotresultuser);
        free(c->results);
    }

    free(c);
}
