#include "nodecrawler.h"
#include "../alloc.h"
#include "log.h"

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
    int                ndispatched;

    // Handle.
    kad_nodecrawlercb_t gotresult;
    void               *gotresultuser;
};

struct n_nodecontext_s
{
    kad_nodecrawler_t *s;
    n_findresults_t   *results;
    kad_id_t           id;
    int                ndispatched;

    // Handle.
    kad_nodecrawlercb_t gotresult;
    void               *gotresultuser;
};

//
// Decls
//

static void kad_nodecrawler_handle(kad_nodecrawler_t *s, n_findresults_t *results, kad_nodecrawlercb_t cb, void *user);
static void kad_nodecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user);
static void kad_nodecrawler_find_unmarked_length(const kad_contact_t *unmarked, void *user);
static void kad_nodecrawler_find_node_cb(bool ok, void *result, void *user);

//
// Public
//

void kad_nodecrawler_init(kad_nodecrawler_t *s, const kad_nodecrawlerargs_t *args)
{
    s->id = args->id;
    s->target = args->target;
    s->capacity = args->capacity;
    s->alpha = args->alpha;
    kad_contactheap_init(&s->nearest, &s->target, args->capacity);
    s->proto = args->proto;
    kad_contactset_init(&s->ignore);

    for (int i = 0; i < args->contacts_size; i++)
    {
        kad_contactheap_push(&s->nearest, &args->contacts[i]);
    }
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
        .ndispatched = 0,

        // Handle.
        .gotresult = cb,
        .gotresultuser = user,
    };

    // Alpha is our request concurrency --- we need to know when we've received all of the responses we were expecting
    // in downstream code. So we get that number here.
    kad_contactheap_unmarked(&s->nearest, kad_nodecrawler_find_unmarked_length, &findcontext.ndispatched);
    findcontext.ndispatched = MIN(findcontext.ndispatched, s->alpha);

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
        for (int j = 0; j < results->data[i].d.find_node.size; j++)
        {
            kad_contact_t *gotcontact = &results->data[i].d.find_node.contacts[j];
            if (!kad_contactset_contains(&s->ignore, &gotcontact->id))
            {
                kad_contactheap_push(&s->nearest, gotcontact);
            }
        }
    }

    if (kad_contactheap_done(&s->nearest))
    {
        kad_contactheap_iter(&s->nearest, cb, data);
        return;
    }

    kad_nodecrawler_find(s, cb, data);
}

void kad_nodecrawler_find_unmarked_length(const kad_contact_t *unmarked, void *user)
{
    int *count = (int *)(user);
    (*count)++;
}

void kad_nodecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user)
{
    n_findcontext_t *fc = (n_findcontext_t *)(user);
    if (fc->i < fc->ndispatched)
    {
        n_nodecontext_t *vc = kad_alloc(1, sizeof(n_nodecontext_t));
        *vc = (n_nodecontext_t){
            .s = fc->s,
            .results = fc->results,
            .id = unmarked->id,
            .ndispatched = fc->ndispatched,
            .gotresult = fc->gotresult,
            .gotresultuser = fc->gotresultuser,
        };

        fc->s->proto->find_node(&(kad_find_node_args_t){
            .self = fc->s->proto,
            .id = &fc->s->id,
            .target_id = &fc->s->target,
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
    c->results->oks = kad_realloc(c->results->oks, c->results->size * sizeof(bool));
    c->results->data = kad_realloc(c->results->data, c->results->size * sizeof(kad_result_t));

    kad_info("find_node: got incremental result\n");

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

    if (c->results->size == c->ndispatched)
    {
        kad_info("find_node: gathered %d results\n", c->ndispatched);
        kad_nodecrawler_handle(c->s, c->results, c->gotresult, c->gotresultuser);
        free(c->results);
    }

    free(c);
}
