#include "logging.h"
#include <munit.h>

extern MunitTest unit_bucket_tests[];
extern MunitTest unit_client_tests[];
extern MunitTest unit_contactheap_tests[];
extern MunitTest unit_contactset_tests[];
extern MunitTest unit_nodecrawler_tests[];
extern MunitTest unit_valuecrawler_tests[];
extern MunitTest unit_ordereddict_tests[];
extern MunitTest unit_protocol_tests[];
extern MunitTest unit_rpc_tests[];
extern MunitTest unit_table_tests[];
extern MunitTest unit_uint256_tests[];

static MunitSuite subsuites[] = {
    {"/bucket", unit_bucket_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/client", unit_client_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/contactheap", unit_contactheap_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/contactset", unit_contactset_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/nodecrawler", unit_nodecrawler_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/valuecrawler", unit_valuecrawler_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/ordereddict", unit_ordereddict_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/protocol", unit_protocol_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/rpc", unit_rpc_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/table", unit_table_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/uint256", unit_uint256_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {NULL, NULL, NULL, 1, MUNIT_SUITE_OPTION_NONE},
};

static MunitSuite suite = {
    NULL, NULL, subsuites, 1, MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char **argv)
{
    kad_logging_set_file(fopen("test.log", "w"));
    kad_logging_set_level(KAD_LL_DEBUG);
    return munit_suite_main(&suite, NULL, argc, argv);
}
