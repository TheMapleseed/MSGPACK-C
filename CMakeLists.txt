cmake_minimum_required(VERSION 3.16)
project(msgpack-c VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MSGPACK_SOURCES
    src/msgpack.c
    src/msgpack_reader.c
)

add_library(msgpack STATIC ${MSGPACK_SOURCES})
set_target_properties(msgpack PROPERTIES
    OUTPUT_NAME "msgpack"
    POSITION_INDEPENDENT_CODE ON
)

add_executable(msgpack_example tests/example.c)
target_link_libraries(msgpack_example msgpack)

add_executable(msgpack_test tests/test_msgpack.c)
target_link_libraries(msgpack_test msgpack)

add_executable(msgpack_test_comprehensive tests/test_comprehensive.c)
target_link_libraries(msgpack_test_comprehensive msgpack)

enable_testing()
add_test(NAME msgpack_test COMMAND msgpack_test)
