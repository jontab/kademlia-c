#include "protocol.h"
#include "alloc.h"
#include "cJSON.h"
#include "log.h"
#include "rpc.h"
#include <assert.h>
#include <stdlib.h>

KAD_GENERATE_LIST_SOURCE(kad_promise_list, kad_promise_t *, free)

static void kad_uv_protocol_ping(const kad_ping_args_t *args);
static void kad_uv_protocol_store(const kad_store_args_t *args);
static void kad_uv_protocol_find_node(const kad_find_node_args_t *args);
static void kad_uv_protocol_find_value(const kad_find_value_args_t *args);
static void kad_uv_protocol_start_recv(kad_uv_protocol_t *self, const char *host, int port);
static void send_request_cb(uv_udp_send_t *req, int status);
static void send_response_cb(uv_udp_send_t *req, int status);
static void recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
static void recv_cb_request(void);
static void uv_buf_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

typedef struct send_context_s send_context_t;

struct send_context_s
{
    kad_uv_protocol_t *self;
    int                request_id;
    void              *user;
    kad_resolve_t      resolve;
    int                type;
};

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

    uv_udp_send_t  *send_req = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_ctx = kad_alloc(1, sizeof(send_context_t));
    send_ctx->self = self;
    send_ctx->user = args->user;
    send_ctx->resolve = args->callback;
    send_ctx->type = KAD_PING;
    send_req->data = (void *)(send_ctx);

    char    *payload = create_ping_request(args->id, &send_ctx->request_id);
    uv_buf_t payload_buf = uv_buf_init(payload, strlen(payload) + 1);

    uv_udp_send(send_req, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_store(const kad_store_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_req = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_ctx = kad_alloc(1, sizeof(send_context_t));
    send_ctx->self = self;
    send_ctx->user = args->user;
    send_ctx->resolve = args->callback;
    send_ctx->type = KAD_STORE;
    send_req->data = (void *)(send_ctx);

    char    *payload = create_store_request(args->id, args->key, args->value, &send_ctx->request_id);
    uv_buf_t payload_buf = uv_buf_init(payload, strlen(payload) + 1);

    uv_udp_send(send_req, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_find_node(const kad_find_node_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_req = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_ctx = kad_alloc(1, sizeof(send_context_t));
    send_ctx->self = self;
    send_ctx->user = args->user;
    send_ctx->resolve = args->callback;
    send_ctx->type = KAD_FIND_NODE;
    send_req->data = (void *)(send_ctx);

    char    *payload = create_find_node_request(args->id, args->target_id, &send_ctx->request_id);
    uv_buf_t payload_buf = uv_buf_init(payload, strlen(payload) + 1);

    uv_udp_send(send_req, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
}

void kad_uv_protocol_find_value(const kad_find_value_args_t *args)
{
    kad_uv_protocol_t *self = (kad_uv_protocol_t *)(args->self);

    struct sockaddr_in addr;
    uv_ip4_addr(args->host, args->port, &addr);

    uv_udp_send_t  *send_req = kad_alloc(1, sizeof(uv_udp_send_t));
    send_context_t *send_ctx = kad_alloc(1, sizeof(send_context_t));
    send_ctx->self = self;
    send_ctx->user = args->user;
    send_ctx->resolve = args->callback;
    send_ctx->type = KAD_FIND_VALUE;
    send_req->data = (void *)(send_ctx);

    char    *payload = create_find_value_request(args->id, args->key, &send_ctx->request_id);
    uv_buf_t payload_buf = uv_buf_init(payload, strlen(payload) + 1);

    uv_udp_send(send_req, &self->socket, &payload_buf, 1, (struct sockaddr *)(&addr), send_request_cb);
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

void send_request_cb(uv_udp_send_t *send_req, int status)
{
    if (status == 0) // OK.
    {
        send_context_t *send_ctx = (send_context_t *)(send_req->data);

        kad_debug("sending request with id: %d\n", send_ctx->request_id);

        kad_promise_t *promise = kad_alloc(1, sizeof(kad_promise_t));
        promise->id = send_ctx->request_id;
        promise->resolve = send_ctx->resolve;
        promise->user = send_ctx->user;
        promise->type = send_ctx->type;

        kad_promise_list_append(&send_ctx->self->promises, promise);
    }
    else
    {
        kad_error("uv_udp_send failed: %s\n", uv_strerror(status));
    }

    free(send_req->data);
    free(send_req);
}

void send_response_cb(uv_udp_send_t *req, int status)
{
    if (status)
    {
        kad_error("send_response_cb: failed to send response: %s\n", uv_strerror(status));
    }
    else
    {
        kad_debug("sent response\n");
    }

    free(req);
}

void recv_cb(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
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

    int request_id;
    if (!kad_payload_request_id(parse_data, &request_id))
    {
        kad_warn("recv_cb: failed to parse request_id\n");
        return;
    }

    if (kad_payload_is_request(parse_data))
    {
        // Request.
        kad_request_t request;
        if (!kad_payload_parse_request(parse_data, &request))
        {
            kad_warn("recv_cb: failed to parse request\n");
            return;
        }

        kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);
        switch (request.type)
        {
        case KAD_PING: {
            char    *response = create_ping_response(&self->table->id, request_id);
            uv_buf_t response_buf = uv_buf_init(response, strlen(response) + 1);

            // TODO: Add to table.
            uv_udp_send_t *send_req = kad_alloc(1, sizeof(uv_udp_send_t));
            uv_udp_send(send_req, handle, &response_buf, 1, addr, send_response_cb);
            break;
        }

        // TODO: Other types.
        default:
            kad_warn("recv_cb: unhandled request type: %d\n", request.type);
            return;
        }

        // Cleanup.
        kad_request_fini(&request, parse_data);
    }
    else
    {
        // Response.
        kad_uv_protocol_t *self = (kad_uv_protocol_t *)(handle->data);
        int                promise_i = -1;

        kad_debug("got response for id: %d\n", request_id);

        for (int i = 0; i < self->promises.size; i++)
        {
            if (self->promises.data[i]->id == request_id)
            {
                promise_i = i;
                break;
            }
        }

        if (promise_i < 0)
        {
            kad_warn("recv_cb: cannot find promise with id: %d\n", request_id);
            return;
        }

        // Execute.
        kad_result_t   result;
        kad_promise_t *promise = self->promises.data[promise_i];
        kad_payload_parse_result(parse_data, promise->type, &result);
        promise->resolve(true, &result, promise->user);
        kad_promise_list_remove(&self->promises, promise_i);

        // Cleanup.
        kad_result_fini(&result, parse_data);
    }
}

void uv_buf_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)(handle);
    buf->base = malloc(suggested_size);
    assert(buf->base && "Out of memory");
    buf->len = suggested_size;
}
