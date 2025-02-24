#include "n5/core/alloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "n5/core/utils.h"

const IAllocator StdAllocVtbl = {
    .alloc = StdAlloc_alloc,
    .free = StdAlloc_free,
};

Allocator StdAlloc_init(void) {
    return &StdAllocVtbl;
}

Block StdAlloc_alloc(Allocator *const self, const AllocInfo *const info) {
    (void)self;
    return (Block) {
        .data = malloc(info->size),
        .size = info->size,
    };
}

void StdAlloc_free(Allocator *const self, const FreeInfo *const info) {
    (void)self;
    free(info->memory.data);
}
