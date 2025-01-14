#include "valuecrawler.h"
#include "../alloc.h"
#include "logging.h"

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/


typedef struct n_findresults_s n_findresults_t;
typedef struct n_findcontext_s n_findcontext_t;
typedef struct n_nodecontext_s n_nodecontext_t;

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct n_findresults_s
{
    kad_id_t     *ids;
    bool         *oks;
    kad_result_t *data;
    int           size;
};

struct n_findcontext_s
{
    kad_valuecrawler_t *s;
    n_findresults_t    *results;
    int                 i;
    int                 ndispatched;

    // Handle.
    kad_valuecrawlercb_t gotvalue;
    void                *gotvalueuser;
};

struct n_nodecontext_s
{
    kad_valuecrawler_t *s;
    n_findresults_t    *results;
    kad_id_t            id;
    int                 ndispatched;

    // Handle.
    kad_valuecrawlercb_t gotvalue;
    void                *gotvalueuser;
};

/******************************************************************************/
/* Decls                                                                      */
/******************************************************************************/

static void kad_valuecrawler_handle(kad_valuecrawler_t *s, n_findresults_t *results, kad_valuecrawlercb_t cb,
                                    void *user);
static void kad_valuecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user);
static void kad_valuecrawler_find_unmarked_length(const kad_contact_t *unmarked, void *user);
static void kad_valuecrawler_find_value_cb(bool ok, void *result, void *user);

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

void kad_valuecrawler_init(kad_valuecrawler_t *s, const kad_valuecrawlerargs_t *args)
{
    s->id = args->id;
    s->key = args->key;
    s->capacity = args->capacity;
    s->alpha = args->alpha;

    kad_id_t target;
    kad_uint256_from_key(args->key, &target);
    kad_contactheap_init(&s->nearest, &target, args->capacity);

    s->proto = args->proto;
    kad_contactset_init(&s->ignore);

    for (int i = 0; i < args->contacts_size; i++)
    {
        kad_contactheap_push(&s->nearest, &args->contacts[i]);
    }
}

void kad_valuecrawler_fini(kad_valuecrawler_t *s)
{
    kad_contactheap_fini(&s->nearest);
    kad_contactset_fini(&s->ignore);
    memset(s, 0, sizeof(*s));
}

void kad_valuecrawler_find(kad_valuecrawler_t *s, kad_valuecrawlercb_t cb, void *user)
{
    n_findcontext_t findcontext = {
        .s = s,
        .results = kad_alloc(1, sizeof(n_findresults_t)),
        .ndispatched = 0,

        // Handle.
        .gotvalue = cb,
        .gotvalueuser = user,
    };

    // Alpha is our request concurrency --- we need to know when we've received all of the responses we were expecting
    // in downstream code. So we get that number here.
    kad_contactheap_unmarked(&s->nearest, kad_valuecrawler_find_unmarked_length, &findcontext.ndispatched);
    findcontext.ndispatched = MIN(findcontext.ndispatched, s->alpha);

    kad_contactheap_unmarked(&s->nearest, kad_valuecrawler_find_unmarked_cb, &findcontext);
}

/******************************************************************************/
/* Statics                                                                    */
/******************************************************************************/

void kad_valuecrawler_handle(kad_valuecrawler_t *s, n_findresults_t *results, kad_valuecrawlercb_t cb, void *data)
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

        // Value.
        if (results->data[i].d.find_value.value)
        {
            cb(results->data[i].d.find_value.value, data);
            return;
        }

        // Contacts.
        for (int j = 0; j < results->data[i].d.find_value.size; j++)
        {
            kad_contact_t *gotcontact = &results->data[i].d.find_value.contacts[j];
            if (!kad_contactset_contains(&s->ignore, &gotcontact->id))
            {
                kad_contactheap_push(&s->nearest, gotcontact);
            }
        }
    }

    if (kad_contactheap_done(&s->nearest))
    {
        cb(NULL, data);
        return;
    }

    kad_valuecrawler_find(s, cb, data);
}

void kad_valuecrawler_find_unmarked_cb(const kad_contact_t *unmarked, void *user)
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
            .gotvalue = fc->gotvalue,
            .gotvalueuser = fc->gotvalueuser,
        };

        fc->s->proto->find_value(&(kad_find_value_args_t){
            .self = fc->s->proto,
            .id = &fc->s->id,
            .key = fc->s->key,
            .host = unmarked->host,
            .port = unmarked->port,
            .callback = kad_valuecrawler_find_value_cb,
            .user = vc,
        });
    }

    fc->i++;
}

void kad_valuecrawler_find_unmarked_length(const kad_contact_t *unmarked, void *user)
{
    int *count = (int *)(user);
    (*count)++;
}

void kad_valuecrawler_find_value_cb(bool ok, void *result, void *user)
{
    n_nodecontext_t *c = (n_nodecontext_t *)(user);

    c->results->size++;
    c->results->ids = kad_realloc(c->results->ids, c->results->size * sizeof(kad_id_t));
    c->results->oks = kad_realloc(c->results->oks, c->results->size * sizeof(bool));
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

    if (c->results->size == c->ndispatched)
    {
        kad_valuecrawler_handle(c->s, c->results, c->gotvalue, c->gotvalueuser);
        free(c->results);
    }

    free(c);
}
