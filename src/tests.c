#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "n5/n5.h"
#include "n5/core/alloc.h"
#include "n5/core/utils.h"

int32_t main(const int32_t argc, const char *const argv[]) {
    printf("Running with %d arg(s):\n", argc);
    for (int32_t i = 0; i < argc; ++i) {
        printf("| %s\n", argv[i]);
    }

    n5_nop();

    printf("\n");

    {
        const char msg[] = "hello there.";
        printf("msg (size: %zu): %s\n", n5_arraySize(msg), msg);
    }

    printf("\n");

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
        struct { int32_t* data; size_t size; } nums = Allocator_createItems(&stdAlloc, int32_t, 8);
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

    return 0;
}
