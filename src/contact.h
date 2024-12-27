#ifndef KADEMLIA_CONTACT_H
#define KADEMLIA_CONTACT_H

#include "uint256.h"
#include <arpa/inet.h>

typedef struct kad_contact_s kad_contact_t;

struct kad_contact_s
{
    kad_uint256_t id;                         // SHA256.
    char          host[INET6_ADDRSTRLEN + 1]; // INET4_ADDRSTRLEN < INET6, + '\0'.
    int           port;
};

void kad_contact_dist(kad_uint256_t *res, const kad_contact_t *a, const kad_contact_t *b);

#endif // KADEMLIA_CONTACT_H
