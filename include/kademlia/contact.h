#ifndef KADEMLIA_CONTACT_H
#define KADEMLIA_CONTACT_H

#include <arpa/inet.h>
#include <kademlia/uint256.h>

typedef struct kad_contact_s kad_contact_t;

struct kad_contact_s
{
    kad_uint256_t id;                // SHA256.
    char host[INET6_ADDRSTRLEN + 1]; // INET4_ADDRSTRLEN < INET6, + '\0'.
    char port[6];                    // XXXXX, + '\0'.
};

void kad_contact_dist(kad_uint256_t *res, const kad_contact_t *a, const kad_contact_t *b);

#endif // KADEMLIA_CONTACT_H
