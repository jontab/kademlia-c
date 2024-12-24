cmake_minimum_required(VERSION 3.5)
project(kademlia)

set(LIBKADEMLIA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bucket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contact.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ordereddict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uint256.c)

set(LIBKADEMLIA_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bucket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contact.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ordereddict.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/table.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uint256.h)

set(TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/munit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_bucket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_ordereddict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_uint256.c)

##
# kademlia-tests
##

add_executable(kademlia-tests
    ${LIBKADEMLIA_SOURCES}
    ${TESTS_SOURCES})

target_include_directories(kademlia-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/munit)

target_compile_options(kademlia-tests PUBLIC
    -O0
    -g
    -DKADEMLIA_NDEBUG)
