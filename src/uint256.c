#include <kademlia/log.h>
#include <kademlia/uint256.h>

int kad_uint256_cmp(const kad_uint256_t *a, const kad_uint256_t *b)
{
    for (int i = 0; i < sizeof(a->d) / sizeof(a->d[0]); i++)
    {
        int cmp = (a->d[i] > b->d[i]) - (a->d[i] < b->d[i]);
        if (cmp)
        {
            return cmp;
        }
    }

    return 0;
}

void kad_uint256_and(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res)
{
    res->d[0] = a->d[0] & b->d[0];
    res->d[1] = a->d[1] & b->d[1];
    res->d[2] = a->d[2] & b->d[2];
    res->d[3] = a->d[3] & b->d[3];
    res->d[4] = a->d[4] & b->d[4];
    res->d[5] = a->d[5] & b->d[5];
    res->d[6] = a->d[6] & b->d[6];
    res->d[7] = a->d[7] & b->d[7];
}

void kad_uint256_xor(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res)
{
    res->d[0] = a->d[0] ^ b->d[0];
    res->d[1] = a->d[1] ^ b->d[1];
    res->d[2] = a->d[2] ^ b->d[2];
    res->d[3] = a->d[3] ^ b->d[3];
    res->d[4] = a->d[4] ^ b->d[4];
    res->d[5] = a->d[5] ^ b->d[5];
    res->d[6] = a->d[6] ^ b->d[6];
    res->d[7] = a->d[7] ^ b->d[7];
}

void kad_uint256_rsh(const kad_uint256_t *a, int amt, kad_uint256_t *out)
{
    out->d[7] = (a->d[7] >> amt) | (a->d[6] << (sizeof(a->d[0]) * 8 - amt));
    out->d[6] = (a->d[6] >> amt) | (a->d[5] << (sizeof(a->d[0]) * 8 - amt));
    out->d[5] = (a->d[5] >> amt) | (a->d[4] << (sizeof(a->d[0]) * 8 - amt));
    out->d[4] = (a->d[4] >> amt) | (a->d[3] << (sizeof(a->d[0]) * 8 - amt));
    out->d[3] = (a->d[3] >> amt) | (a->d[2] << (sizeof(a->d[0]) * 8 - amt));
    out->d[2] = (a->d[2] >> amt) | (a->d[1] << (sizeof(a->d[0]) * 8 - amt));
    out->d[1] = (a->d[1] >> amt) | (a->d[0] << (sizeof(a->d[0]) * 8 - amt));
    out->d[0] = (a->d[0] >> amt);
}

void kad_uint256_lsh(const kad_uint256_t *a, int amt, kad_uint256_t *out)
{
    out->d[0] = (a->d[0] << amt) | (a->d[1] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[1] = (a->d[1] << amt) | (a->d[2] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[2] = (a->d[2] << amt) | (a->d[3] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[3] = (a->d[3] << amt) | (a->d[4] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[4] = (a->d[4] << amt) | (a->d[5] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[5] = (a->d[5] << amt) | (a->d[6] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[6] = (a->d[6] << amt) | (a->d[7] >> (sizeof(a->d[0]) * 8 - amt));
    out->d[7] = (a->d[7] << amt);
}

void kad_uint256_add(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res)
{
    // https://stackoverflow.com/questions/365522/what-is-the-best-way-to-add-two-numbers-without-using-the-operator.
    kad_uint256_t c; // a.
    kad_uint256_t d; // b.
    kad_uint256_t x = *a;
    kad_uint256_t y = *b;
    do
    {
        kad_uint256_and(&x, &y, &c); // a = x & y.
        kad_uint256_xor(&x, &y, &d); // b = x ^ y.
        kad_uint256_lsh(&c, 1, &x);  // x = a << 1.
        y = d;                       // y = b;
    } while (!kad_uint256_iszero(&c));
    *res = d;
}

void kad_uint256_avg(const kad_uint256_t *a, const kad_uint256_t *b, kad_uint256_t *res)
{
    // (A + B)     = 2 * (A & B) + (A ^ B).
    // (A + B) / 2 =     (A & B) + (A ^ B) / 2.
    //                   ___L___   ___R___.
    //                             _______RR__.
    //                   _______RES___________.
    kad_uint256_t l;
    kad_uint256_t r;
    kad_uint256_t rr;
    kad_uint256_and(a, b, &l);
    kad_uint256_xor(a, b, &r);
    kad_uint256_rsh(&r, 1, &rr);
    kad_uint256_add(&l, &rr, res);
}

bool kad_uint256_iszero(const kad_uint256_t *a)
{
    uint32_t res = 0;
    res |= a->d[0];
    res |= a->d[1];
    res |= a->d[2];
    res |= a->d[3];
    res |= a->d[4];
    res |= a->d[5];
    res |= a->d[6];
    res |= a->d[7];
    return res == 0;
}
