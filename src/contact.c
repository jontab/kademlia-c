#include "contact.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void kad_contact_dist(kad_uint256_t *res, const kad_contact_t *a, const kad_contact_t *b)
{
    kad_uint256_xor(&a->id, &b->id, res);
}
