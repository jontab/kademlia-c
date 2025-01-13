#include "logging.h"
#include "uint256.h"
#include <munit.h>

MunitResult unit_uint256_cmp(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {0, 0, 64, 0, 0, 0, 0, 0};
    kad_uint256_t b = {0, 0, 0, 0, 0, 64, 0, 0};
    munit_assert_int(kad_uint256_cmp(&a, &b), >, 0);
    munit_assert_int(kad_uint256_cmp(&b, &a), <, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_and(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {0, 0, 1, 0, 1, 0, 0, 0};
    kad_uint256_t b = {0, 0, 1, 1, 0, 0, 0, 0};
    kad_uint256_t c = {0, 0, 1, 0, 0, 0, 0, 0};
    kad_uint256_t res;
    kad_uint256_and(&a, &b, &res);
    munit_assert_int(kad_uint256_cmp(&res, &c), ==, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_xor(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {0, 0, 1, 0, 1, 0, 0, 0};
    kad_uint256_t b = {0, 0, 1, 1, 0, 0, 0, 0};
    kad_uint256_t c = {0, 0, 0, 1, 1, 0, 0, 0};
    kad_uint256_t res;
    kad_uint256_xor(&a, &b, &res);
    munit_assert_int(kad_uint256_cmp(&res, &c), ==, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_rsh(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {1, 0x08844000, 0, 0, 0, 0, 0, 2};
    kad_uint256_t b = {0, 0x84422000, 0, 0, 0, 0, 0, 1};
    kad_uint256_t c = {0, 0x42211000, 0, 0, 0, 0, 0, 0};

    kad_uint256_t res1;
    kad_uint256_t res2;
    kad_uint256_rsh(&a, 1, &res1);
    kad_uint256_rsh(&a, 2, &res2);
    munit_assert_int(kad_uint256_cmp(&res1, &b), ==, 0);
    munit_assert_int(kad_uint256_cmp(&res2, &c), ==, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_lsh(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {1, 0x42211000, 0, 0, 0, 0, 0, 1};
    kad_uint256_t b = {2, 0x84422000, 0, 0, 0, 0, 0, 2};
    kad_uint256_t c = {5, 0x08844000, 0, 0, 0, 0, 0, 4};

    kad_uint256_t res1;
    kad_uint256_t res2;
    kad_uint256_lsh(&a, 1, &res1);
    kad_uint256_lsh(&a, 2, &res2);
    munit_assert_int(kad_uint256_cmp(&res1, &b), ==, 0);
    munit_assert_int(kad_uint256_cmp(&res2, &c), ==, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_add(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {0, 0x82222222, 0, 0, 0, 3, 2, 1};
    kad_uint256_t b = {0, 0x81111111, 0, 0, 0, 4, 3, 2};
    kad_uint256_t c = {1, 0x03333333, 0, 0, 0, 7, 5, 3};

    kad_uint256_t res;
    kad_uint256_add(&a, &b, &res);
    munit_assert_int(kad_uint256_cmp(&res, &c), ==, 0);
    return MUNIT_OK;
}

MunitResult unit_uint256_avg(const MunitParameter params[], void *data)
{
    kad_uint256_t a = {0, 0, 0, 40, 0, 0, 0, 0};
    kad_uint256_t b = {0, 0, 0, 20, 0, 0, 0, 0};
    kad_uint256_t c = {0, 0, 0, 30, 0, 0, 0, 0};

    kad_uint256_t res;
    kad_uint256_avg(&a, &b, &res);
    munit_assert_int(kad_uint256_cmp(&res, &c), ==, 0);
    return MUNIT_OK;
}

MunitTest unit_uint256_tests[] = {
    {"/cmp", unit_uint256_cmp, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/and", unit_uint256_and, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/xor", unit_uint256_xor, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/rsh", unit_uint256_rsh, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/lsh", unit_uint256_lsh, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/add", unit_uint256_add, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/avg", unit_uint256_avg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
