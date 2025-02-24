#include <stdint.h>
#include <stdio.h>

#include "n5/n5.h"

int32_t main(const int32_t argc, const char *const argv[]) {
    printf("Running with %d arg(s):\n", argc);
    for (int32_t i = 0; i < argc; ++i) {
        printf("| %s\n", argv[i]);
    }

    n5_nop();

    return 0;
}
