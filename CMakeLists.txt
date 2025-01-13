cmake_minimum_required(VERSION 3.5)
project(kademlia)

set(LIBKADEMLIA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/contactheap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/nodecrawler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/valuecrawler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/contactlist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/contactset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/ordereddict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bucket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cJSON.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contact.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/storage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uint256.c)

set(LIBKADEMLIA_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/contactheap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/nodecrawler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crawlers/valuecrawler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/contactlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/contactset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ds/ordereddict.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alloc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bucket.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cJSON.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contact.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/storage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/table.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uint256.h)

set(TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/munit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_bucket.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_client.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_contactheap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_contactset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_nodecrawler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_valuecrawler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_ordereddict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_rpc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_uint256.c)

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)

find_library(LIBUV_LIBRARIES NAMES uv libuv)

find_package(OpenSSL REQUIRED)

##
# kademlia-tests
##

add_executable(kademlia-tests
    ${LIBKADEMLIA_SOURCES}
    ${TESTS_SOURCES})

target_include_directories(kademlia-tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/munit
    ${LIBUV_INCLUDE_DIR})

target_link_libraries(kademlia-tests
    ${LIBUV_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto)

target_compile_options(kademlia-tests PUBLIC
    -O0
    -g
    -DKADEMLIA_NDEBUG)
