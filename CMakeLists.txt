cmake_minimum_required(VERSION 3.25)

project(
    n5
    VERSION 0.1.0
    LANGUAGES C
)

add_library(n5 STATIC)

add_executable(tests)
target_link_libraries(tests PRIVATE n5)

set_target_properties(
    n5 tests
    PROPERTIES
        C_STANDARD 11
)

target_compile_options(
    n5 PRIVATE
    $<IF:$<C_COMPILER_ID:MSVC>,
        /WX /W4,
        -Werror -Wall -Wextra -Wpedantic>
)
target_compile_options(
    tests PRIVATE
    $<IF:$<C_COMPILER_ID:MSVC>,
        /WX /W4,
        -Werror -Wall -Wextra -Wpedantic>
)

target_sources(
    n5
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS include
            FILES
                include/n5/n5.h
                include/n5/core/alloc.h
                include/n5/core/utils.h
    PRIVATE
        src/n5/n5.c
        src/n5/core/alloc.c
)

target_sources(
    tests
    PRIVATE
        src/tests.c
)
