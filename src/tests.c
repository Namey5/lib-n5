#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "n5/alloc.h"
#include "n5/slice.h"
#include "n5/string.h"
#include "n5/utils.h"

int32_t main(const int32_t argc, const char *const argv[]) {
    printf("Running with %d arg(s):\n", argc);
    for (int32_t i = 0; i < argc; ++i) {
        printf("| %s\n", argv[i]);
    }

    TestAlloc mainAlloc = TestAlloc_init();

    printf("\n");

    {
        const char msg[] = "hello there.";
        printf("msg (size: %zu): %s\n", n5_arraySize(msg), msg);
    }

    printf("\n");

    {
        Allocator stdAlloc = StdAlloc_init();
        {
            Block memory = Allocator_alloc(&stdAlloc, char, 32);
            char* buffer = memory.data;
            buffer[0] = '\0';
            strncat(buffer, "hello", 32);
            printf("StdAlloc - buffer: %s\n", buffer);
            Allocator_free(&stdAlloc, memory);
        }
        {
            int32_t* num = Allocator_createItem(&stdAlloc, int32_t);
            *num = 17;
            printf("StdAlloc - num (%p): %d\n", (void*)num, *num);
            Allocator_destroyItem(&stdAlloc, num);
        }
        {
            Slice(int32_t) nums = Allocator_createItems(&stdAlloc, int32_t, 8);
            for (size_t i = 0; i < nums.size; ++i) {
                nums.data[i] = i * 4;
            }

            printf("StdAlloc - nums (size: %zu): ", nums.size);
            for (size_t i = 0; i < nums.size; ++i) {
                printf("%d, ", nums.data[i]);
            }
            printf("\n");

            Allocator_destroyItems(&stdAlloc, nums);
        }
    }

    printf("\n");

    {
        const size_t arenaSize = 512;

        Arena arena;
        bool success = Arena_init(&arena, &mainAlloc.base, arenaSize);
        assert(success);

        printf("Arena (%p, size: %zu):\n", arena.pool.data, arena.pool.size);
        for (int32_t i = 0; i < 32; ++i) {
            if (i % 16 == 0) {
                printf("| -- RESETTING --\n");
                Arena_reset(&arena);
            }

            int32_t* num = Allocator_createItem(&arena.base, int32_t);
            *num = i * 4;
            printf("| offset %zu: num (%p): %d\n", arena.offset, (void*)num, *num);

            if (i % 4 == 0) {
                printf("| - freeing\n");
                Allocator_destroyItem(&arena.base, num);
            }
        }

        Arena_deinit(&arena);
    }

    printf("\n");

    {
        TestAlloc testAlloc = TestAlloc_init();

        Slice(int32_t*) nums = Allocator_createItems(&testAlloc.base, int32_t*, 8);

        printf("TestAlloc - %zu int pointers (%p):\n", nums.size, (void*)nums.data);
        for (size_t i = 0; i < nums.size; ++i) {
            nums.data[i] = Allocator_createItem(&testAlloc.base, int32_t);
            *nums.data[i] = i * 4;
            printf("| %zu (%p): %d\n", i + 1, (void*)nums.data[i], *nums.data[i]);
        }

        printf("Freeing int pointers (%p):\n", (void*)nums.data);
        for (size_t i = 0; i < nums.size; ++i) {
            if (i % 2 == 0) {
                printf("| %zu: %p", i + 1, (void*)nums.data[i]);
                if (i % 4 == 2) {
                    printf(" (twice)");
                    Allocator_destroyItem(&testAlloc.base, nums.data[i]);
                }
                printf("\n");
                Allocator_destroyItem(&testAlloc.base, nums.data[i]);
            }
        }
        Allocator_destroyItems(&testAlloc.base, nums);

        TestAlloc_deinit(&testAlloc);
    }

    printf("\n");

    {
        cstr literal = cstr_literal("This is a literal string.");
        printf("cstr_literal (size: %zu): %s\n", literal.size, literal.data);

        cstr substring = cstr_slice(literal, 5, 12);
        printf("cstr slice (size: %zu): ", substring.size);
        for (const char* c = Slice_start(substring); c < Slice_end(substring); ++c) {
            printf("%c", *c);
        }
        printf("\n");

        str local = str_local("Hello, slices!");
        local.data[3] = '7';
        printf("str_local (size: %zu): %s\n", local.size, local.data);

        local.data[3] = 'l';
        literal = cstr_literal("Hello, slices!");
        printf(
            "local '%s' == literal '%s': %d\n",
            local.data,
            literal.data,
            Slice_compare(cstr_cast(local), literal)
        );

        String dynamicString = String_from(&mainAlloc.base, cstr_literal("Hello"));
        printf(
            "String_from (capacity: %zu, size: %zu): %s\n",
            dynamicString.capacity,
            dynamicString.str.size,
            dynamicString.str.data
        );
        String_append_str(&dynamicString, cstr_literal(" there!"));
        printf(
            "String_concat (capacity: %zu, size: %zu): %s\n",
            dynamicString.capacity,
            dynamicString.str.size,
            dynamicString.str.data
        );
        for (int32_t i = 0; i < 4; ++i) {
            String_append_str(&dynamicString, cstr_slice(dynamicString.str, 2, dynamicString.str.size / 2 - 1));
            printf(
                "String_concat recursive #%d (capacity: %zu, size: %zu): %s\n",
                i + 1,
                dynamicString.capacity,
                dynamicString.str.size,
                dynamicString.str.data
            );
        }

        String_format(
            &dynamicString,
            cstr_literal("This is a {str} string with some {str} {f32}."),
            cstr_literal("FORMAT"),
            cstr_literal("STUFF"),
            -432.56428678
        );
        printf(
            "String_format (capacity: %zu, size: %zu): %s\n",
            dynamicString.capacity,
            dynamicString.str.size,
            dynamicString.str.data
        );

        String_free(&dynamicString);
    }

    TestAlloc_deinit(&mainAlloc);

    return 0;
}
