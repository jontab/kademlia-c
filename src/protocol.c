#include "protocol.h"
#include <assert.h>
#include <stdlib.h>
#include <uv.h>

struct kad_protocol_s
{
    uv_loop_t *loop;
    uv_udp_t recv_socket;
    uv_udp_t send_socket;
};

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buffer);
static void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);

//
// Public
//

void kad_protocol_init(kad_protocol_t *s, uv_loop_t *loop, const char *host, int port)
{
    memset(s, 0, sizeof(*s));
    s->loop = loop;

    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr;
    uv_ip4_addr(host, port, &recv_addr);
    uv_ip4_addr(host, port, &send_addr);

    uv_udp_init(loop, &s->recv_socket);
    uv_udp_init(loop, &s->send_socket);

    uv_udp_bind(&s->recv_socket, (struct sockaddr *)(&recv_addr), 0);
    uv_udp_bind(&s->send_socket, (struct sockaddr *)(&send_addr), 0);

    s->recv_socket.data = (void *)(s);
    uv_udp_recv_start(&s->recv_socket, alloc_buffer, on_read);
}

void kad_protocol_fini(kad_protocol_t *s)
{
    uv_udp_recv_stop(&s->recv_socket);
}

//
// Static
//

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buffer)
{
    char *buf = malloc(suggested_size);
    assert(buf && "Out of memory");
    *buffer = uv_buf_init(buf, suggested_size);
}

void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
}
