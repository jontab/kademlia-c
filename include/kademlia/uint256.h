#ifndef KADEMLIA_UINT256_H
#define KADEMLIA_UINT256_H

#include <stdbool.h>
#include <stdint.h>

typedef struct kad_uint256_s kad_uint256_t;

struct kad_uint256_s
{
    uint32_t d[256 / 32];
};

typedef const kad_uint256_t kad_id_t;

// TODO: Consider using a library for this.
int kad_uint256_cmp(const kad_uint256_t *a, const kad_uint256_t *b);
void kad_uint256_and(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_xor(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_rsh(const kad_uint256_t *a, int amt, kad_uint256_t *out);
void kad_uint256_lsh(const kad_uint256_t *a, int amt, kad_uint256_t *out);
void kad_uint256_add(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_avg(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
bool kad_uint256_iszero(const kad_uint256_t *a);

#endif // KADEMLIA_UINT256_H
