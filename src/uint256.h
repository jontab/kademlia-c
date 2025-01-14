#ifndef KADEMLIA_UINT256_H
#define KADEMLIA_UINT256_H

#include <stdbool.h>
#include <stdint.h>

/******************************************************************************/
/* Typedefs                                                                   */
/******************************************************************************/

typedef struct kad_uint256_s kad_uint256_t;
typedef kad_uint256_t        kad_id_t;

/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

struct kad_uint256_s
{
    uint32_t d[256 / 32];
};

/******************************************************************************/
/* Public                                                                     */
/******************************************************************************/

int  kad_uint256_cmp(const kad_uint256_t *a, const kad_uint256_t *b);
void kad_uint256_and(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_xor(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_rsh(const kad_uint256_t *a, int amt, kad_uint256_t *out);
void kad_uint256_lsh(const kad_uint256_t *a, int amt, kad_uint256_t *out);
void kad_uint256_add(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
void kad_uint256_avg(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res);
bool kad_uint256_iszero(const kad_uint256_t *a);
void kad_uint256_from_key(const char *key, kad_uint256_t *out);

#endif // KADEMLIA_UINT256_H
