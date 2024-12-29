#include "protocol.h"
#include "alloc.h"
#include "cJSON.h"
#include "log.h"
#include "rpc.h"
#include <assert.h>
#include <stdlib.h>

KAD_GENERATE_LIST_SOURCE(kad_promise_list, kad_promise_t *, free)

typedef struct send_context_s            send_context_t;
typedef struct send_response_context_s   send_response_context_t;
typedef struct request_timeout_context_s request_timeout_context_t;

struct send_context_s
{
    kad_uv_protocol_t *self;
    int                request_id;
    void              *user;
    kad_resolve_t      resolve;
    int                type;
    char              *payload_str;
};

struct send_response_context_s
{
    char *payload_str;
};

struct request_timeout_context_s
{
    int                 request_id;
    kad_promise_list_t *promises;
};

static void kad_uv_protocol_ping(const kad_ping_args_t *args);
static void kad_uv_protocol_store(const kad_store_args_t *args);
static void kad_uv_protocol_find_node(const kad_find_node_args_t *args);
static void kad_uv_protocol_find_value(const kad_find_value_args_t *args);
static void kad_uv_protocol_start_recv(kad_uv_protocol_t *self, const char *host, int port);
static int  find_promise_index(kad_promise_list_t *promises, int request_id);
static void send_request_cb(uv_udp_send_t *send_request, int status);
static void send_request_cb_register_promise(send_context_t *send_context);
static void send_response_cb(uv_udp_send_t *req, int status);
static void recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
static void recv_cb_request(uv_udp_t *handle, void *parse_data, const struct sockaddr *addr, int request_id);
static void recv_cb_request_add_contact(kad_uv_protocol_t *self, kad_id_t *id, const struct sockaddr *addr);
static void recv_cb_request_ping(uv_udp_t *handle, kad_request_t *req, const struct sockaddr *addr, int req_id);
static void recv_cb_request_store(uv_udp_t *handle, kad_request_t *req, const struct sockaddr *addr, int req_id);
static void recv_cb_response(uv_udp_t *handle, void *parse_data, int request_id);
static void request_timeout_cb(uv_timer_t *handle);
static void request_timeout_start(send_context_t *context);
static void uv_buf_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

//
// Public
//

kad_uv_protocol_t *kad_uv_protocol_new(uv_loop_t *loop, kad_table_t *table)
{
    kad_uv_protocol_t *self = kad_alloc(1, sizeof(kad_uv_protocol_t));
    self->base = (kad_protocol_t){
        .ping = kad_uv_protocol_ping,
        .store = kad_uv_protocol_store,
        .find_node = kad_uv_protocol_find_node,
        .find_value = kad_uv_protocol_find_value,
    };
    self->loop = loop;
    self->table = table;
    return self;
}

void kad_uv_protocol_free(kad_uv_protocol_t *self)
{
    kad_promise_list_fini(&self->promises);
    uv_udp_recv_stop(&self->socket);
}

void kad_uv_protocol_start(kad_uv_protocol_t *self, const char *host, int port)
{
    kad_uv_protocol_start_recv(self, host, port);
}

//
// Static
//

void kad_uv_protocol_ping(const kad_ping_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_context = kad_alloc(1, sizeof(send_context_t));

    send_context->self = self;
    send_context->user = args->user;
    send_context->resolve = args->callback;
    send_context->type = KAD_PING;
    send_context->payload_str = create_ping_request(args->id, &send_context->request_id);

    send_request->data = (void *)(send_context);

    uv_buf_t payload_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_store(const kad_store_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_context = kad_alloc(1, sizeof(send_context_t));

    send_context->self = self;
    send_context->user = args->user;
    send_context->resolve = args->callback;
    send_context->type = KAD_STORE;
    send_context->payload_str = create_store_request(args->id, args->key, args->value, &send_context->request_id);

    send_request->data = (void *)(send_context);

    uv_buf_t payload_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_find_node(const kad_find_node_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_context = kad_alloc(1, sizeof(send_context_t));

    send_context->self = self;
    send_context->user = args->user;
    send_context->resolve = args->callback;
    send_context->type = KAD_FIND_NODE;
    send_context->payload_str = create_find_node_request(args->id, args->target_id, &send_context->request_id);

    send_request->data = (void *)(send_context);

    uv_buf_t payload_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_find_value(const kad_find_value_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_context = kad_alloc(1, sizeof(send_context_t));

    send_context->self = self;
    send_context->user = args->user;
    send_context->resolve = args->callback;
    send_context->type = KAD_FIND_VALUE;
    send_context->payload_str = create_find_value_request(args->id, args->key, &send_context->request_id);

    send_request->data = (void *)(send_context);

    uv_buf_t payload_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_start_recv(kad_uv_protocol_t *self, const char *host, int port)
{
    struct sockaddr_in recv_addr;
    uv_ip4_addr(host, port, &recv_addr);

    uv_udp_init(self->loop, &self->socket);
    uv_udp_bind(&self->socket, (struct sockaddr *)(&recv_addr), 0);
    self->socket.data = (void *)(self);

    uv_udp_recv_start(&self->socket, uv_buf_alloc, recv_cb);
}

int find_promise_index(kad_promise_list_t *promises, int request_id)
{
    for (int i = 0; i < promises->size; i++)
    {
        if (request_id == promises->data[i]->id)
        {
            return i;
        }
    }

    return -1;
}

void send_request_cb(uv_udp_send_t *send_request, int status)
{
    send_context_t *send_context = (send_context_t *)(send_request->data);
    if (status == 0) // OK.
    {
        send_request_cb_register_promise(send_context);
        request_timeout_start(send_context);
    }
    else
    {
        kad_error("uv_udp_send failed: %s\n", uv_strerror(status));
    }

    free(send_context->payload_str);
    free(send_context);
    free(send_request);
}

void send_request_cb_register_promise(struct send_context_s *send_context)
{
    kad_promise_t *promise = kad_alloc(1, sizeof(kad_promise_t));
    promise->id = send_context->request_id;
    promise->resolve = send_context->resolve;
    promise->user = send_context->user;
    promise->type = send_context->type;
    kad_promise_list_append(&send_context->self->promises, promise);
}

void send_response_cb(uv_udp_send_t *send_request, int status)
{
    send_response_context_t *send_context = (send_response_context_t *)(send_request->data);
    if (status)
    {
        kad_error("send_response_cb: failed to send response: %s\n", uv_strerror(status));
    }

    free(send_context->payload_str);
    free(send_context);
    free(send_request);
}

void recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);
    if (nread < 0)
    {
        kad_warn("recv_cb: negative nread\n");
        return;
    }

    if (nread == 0)
    {
        return;
    }

    // Debug.
    if (addr->sa_family == AF_INET)
    {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)(addr);
        char                addr_host[BUFSIZ] = {0};
        int                 addr_port = ntohs(addr_in->sin_port);
        uv_ip4_name((struct sockaddr_in *)(addr_in), addr_host, sizeof(addr_host) - 1);
        kad_debug("got data from %s:%d\n", addr_host, addr_port);
    }
    else
    {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)(addr);
        char                 addr_host[BUFSIZ] = {0};
        int                  addr_port = ntohs(addr_in6->sin6_port);
        uv_ip6_name((struct sockaddr_in6 *)(addr), addr_host, sizeof(addr_host) - 1);
        kad_debug("got data from [%s]:%d\n", addr_host, addr_port);
    }

    void *parse_data = kad_payload_parse(buf->base, nread);
    if (!parse_data)
    {
        kad_warn("recv_cb: failed to parse payload\n");
        return;
    }

    // Id.
    int request_id;
    if (!kad_payload_request_id(parse_data, &request_id))
    {
        kad_warn("recv_cb: failed to parse request_id\n");
        kad_result_fini(NULL, parse_data);
        return;
    }

    // Execute.
    if (kad_payload_is_request(parse_data))
    {
        recv_cb_request(handle, parse_data, addr, request_id);
    }
    else
    {
        recv_cb_response(handle, parse_data, request_id);
    }
}

void recv_cb_request(uv_udp_t *handle, void *parse_data, const struct sockaddr *addr, int request_id)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);

    // Request.
    kad_request_t request;
    if (!kad_payload_parse_request(parse_data, &request))
    {
        kad_warn("recv_cb_request: failed to parse request\n");
        kad_request_fini(NULL, parse_data);
        return;
    }

    switch (request.type)
    {
    case KAD_PING:
        recv_cb_request_ping(handle, &request, addr, request_id);
        break;
    case KAD_STORE:
        recv_cb_request_store(handle, &request, addr, request_id);
        break;

    // TODO: Other types.
    default:
        kad_warn("recv_cb_request: unhandled request type: %d\n", request.type);
        goto cleanup;
    }

cleanup:
    kad_request_fini(&request, parse_data);
}

void recv_cb_request_add_contact(kad_uv_protocol_t *self, kad_id_t *id, const struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
    {
        const struct sockaddr_in *addr_in = (const struct sockaddr_in *)(addr);

        // Sender.
        kad_contact_t sender = {0};
        sender.id = *id;
        sender.port = addr_in->sin_port;
        uv_ip4_name(addr_in, sender.host, sizeof(sender.host) - 1);

        // Add to table.
        kad_table_add_contact(self->table, &sender);
    }
}

void recv_cb_request_ping(uv_udp_t *handle, kad_request_t *req, const struct sockaddr *addr, int req_id)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);

    // Setup.
    uv_udp_send_t           *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_response_context_t *send_context = kad_alloc(1, sizeof(send_response_context_t));

    send_context->payload_str = create_ping_response(&self->table->id, req_id);

    send_request->data = (void *)(send_context);

    // Execute.
    recv_cb_request_add_contact(self, &req->d.ping.id, addr);

    // Send.
    uv_buf_t response_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, handle, &response_buf, 1, addr, send_response_cb);
}

void recv_cb_request_store(uv_udp_t *handle, kad_request_t *req, const struct sockaddr *addr, int req_id)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);

    // Setup.
    uv_udp_send_t           *send_request = kad_alloc(1, sizeof(uv_udp_send_t));
    send_response_context_t *send_context = kad_alloc(1, sizeof(send_response_context_t));

    send_context->payload_str = create_store_response(req_id);

    send_request->data = (void *)(send_context);

    // Execute.
    recv_cb_request_add_contact(self, &req->d.store.id, addr);

    //
    // TODO: Do the actual storing.
    //

    // Send.
    uv_buf_t response_buf = uv_buf_init(send_context->payload_str, strlen(send_context->payload_str) + 1);
    uv_udp_send(send_request, handle, &response_buf, 1, addr, send_response_cb);
}

void recv_cb_response(uv_udp_t *handle, void *parse_data, int request_id)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);

    // Type.
    int promise_idx = find_promise_index(&self->promises, request_id);
    if (promise_idx < 0)
    {
        kad_warn("recv_cb_response: cannot find promise with id: %d\n", request_id);
        kad_result_fini(NULL, parse_data);
        return;
    }

    kad_promise_t     *promise = self->promises.data[promise_idx];
    kad_request_type_t request_type = promise->type;

    // Result.
    kad_result_t result;
    if (!kad_payload_parse_result(parse_data, request_type, &result))
    {
        kad_warn("recv_cb_response: failed to parse result\n");
        kad_result_fini(NULL, parse_data);
        return;
    }

    // Execute.
    promise->resolve(true, &result, promise->user);
    kad_promise_list_remove(&self->promises, promise_idx);
    kad_result_fini(&result, parse_data);
}

void request_timeout_cb(uv_timer_t *handle)
{
    request_timeout_context_t *timeout_context = (request_timeout_context_t *)(handle->data);

    int                 request_id = timeout_context->request_id;
    kad_promise_list_t *promises = timeout_context->promises;

    int promise_idx = find_promise_index(promises, request_id);
    if (promise_idx >= 0)
    {
        kad_promise_t *promise = promises->data[promise_idx];
        promise->resolve(false, NULL, promise->user);
        kad_promise_list_remove(promises, promise_idx);
    }

    free(timeout_context);
}

void request_timeout_start(send_context_t *context)
{
    uv_timer_t                *timeout = kad_alloc(1, sizeof(uv_timer_t));
    request_timeout_context_t *timeout_context = kad_alloc(1, sizeof(request_timeout_context_t));

    timeout_context->request_id = context->request_id;
    timeout_context->promises = &context->self->promises;

    timeout->data = (void *)(timeout_context);

    uv_timer_init(context->self->loop, timeout);
    uv_timer_start(timeout, request_timeout_cb, 5000, 0);
}

void uv_buf_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = malloc(suggested_size);
    assert(buf->base && "Out of memory");
    buf->len = suggested_size;
}
