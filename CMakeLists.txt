cmake_minimum_required(VERSION 3.5)
project(kademlia)

##
# kademlia-static
##

# add_library(kademlia-static STATIC
#     src/bucket.c
#     src/contact.c
#     src/log.c
#     src/ordereddict.c
#     src/uint256.c)

# target_include_directories(kademlia-static PUBLIC
#     include)

# target_compile_options(kademlia-static PUBLIC
#     -O0
#     -g)

##
# kademlia-tests
##

add_executable(kademlia-tests
    src/bucket.c
    src/contact.c
    src/log.c
    src/ordereddict.c
    src/table.c
    src/uint256.c
    tests/main.c
    tests/munit.c
    tests/unit_bucket.c
    tests/unit_ordereddict.c
    tests/unit_table.c
    tests/unit_uint256.c)

# target_link_libraries(kademlia-tests
#     kademlia-static)

target_include_directories(kademlia-tests PUBLIC
    deps/munit
    include)

target_compile_options(kademlia-tests PUBLIC
    -O0
    -g
    -DKADEMLIA_NDEBUG)
