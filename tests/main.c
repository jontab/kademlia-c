#include <munit.h>

extern MunitTest unit_bucket_tests[];
extern MunitTest unit_ordereddict_tests[];
extern MunitTest unit_table_tests[];
extern MunitTest unit_uint256_tests[];

static MunitSuite subsuites[] = {
    {"/bucket", unit_bucket_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/ordereddict", unit_ordereddict_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/table", unit_table_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/uint256", unit_uint256_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {NULL, NULL, NULL, 1, MUNIT_SUITE_OPTION_NONE},
};

static MunitSuite suite = {
    NULL, NULL, subsuites, 1, MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char **argv)
{
    return munit_suite_main(&suite, NULL, argc, argv);
}
