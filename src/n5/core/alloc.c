#include "n5/core/alloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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

const IAllocator ArenaVtbl = {
    .alloc = Arena_alloc,
    .free = Arena_free,
};

bool Arena_init(Arena *const self, Allocator *const owner, const size_t size) {
    assert(self != NULL);
    assert(owner != NULL);
    assert(size > 0);

    Block pool = Allocator_alloc(owner, uint8_t, size);
    if (pool.data == NULL) {
        return false;
    }

    *self = (Arena) {
        .base = &ArenaVtbl,
        .owner = owner,
        .pool = pool,
    };

    return true;
}

void Arena_deinit(Arena *const self) {
    assert(self != NULL);
    assert(self->owner != NULL);
    Allocator_free(self->owner, self->pool);
}

void Arena_reset(Arena *const self) {
    assert(self != NULL);
    self->offset = 0;
}

Block Arena_alloc(Allocator *const base, const AllocInfo *const info) {
    Arena *const self = (Arena*)base;
    assert(self->pool.data != NULL);

    void *const poolStart = (uint8_t*)self->pool.data + self->offset;
    void *const poolEnd = (uint8_t*)self->pool.data + self->pool.size;

    void *const memoryStart = n5_align(poolStart, info->align);
    void *const memoryEnd = (uint8_t*)memoryStart + info->size;

    Block memory = { 0 };
    if (memoryEnd > poolEnd) {
        return memory;
    }

    self->offset = (uintptr_t)memoryEnd - (uintptr_t)self->pool.data;

    memory.data = memoryStart;
    memory.size = info->size;

    return memory;
}

void Arena_free(Allocator *const base, const FreeInfo *const info) {
    Arena *const self = (Arena*)base;

    void *const poolStart = (uint8_t*)self->pool.data + self->offset;
    void *const memoryEnd = (uint8_t*)info->memory.data + info->memory.size;
    if (poolStart == memoryEnd) {
        self->offset -= info->memory.size;
    }
}

const IAllocator TestAllocVtbl = {
    .alloc = TestAlloc_alloc,
    .free = TestAlloc_free,
};

TestAlloc TestAlloc_init(void) {
    return (TestAlloc) {
        .base = &TestAllocVtbl,
        .head = NULL,
    };
}

void TestAlloc_deinit(TestAlloc *const self) {
    assert(self != NULL);

    TestAllocHeader* header = self->head;
    while (header != NULL) {
        fprintf(
            stderr,
            "[TestAlloc] Memory leak detected: %s (line %zu, address %p)\n",
            header->debugInfo.file,
            header->debugInfo.line,
            (void*)(header + 1)
        );

        TestAllocHeader* next = header->next;
        StdAlloc_free(NULL, &(FreeInfo) {
            .memory = {
                .data = header,
                .size = header->size,
            },
        });
        header = next;
    }
}

Block TestAlloc_alloc(Allocator *const base, const AllocInfo *const info) {
    TestAlloc *const self = (TestAlloc*)base;

    Block memory = StdAlloc_alloc(NULL, &(AllocInfo) {
        .size = info->size + sizeof(TestAllocHeader),
    });

    if (memory.data == NULL) {
        return memory;
    }

    TestAllocHeader *const header = memory.data;
    header->size = memory.size;
    header->debugInfo = info->debugInfo;
    header->next = self->head;

    self->head = header;

    memory.data = header + 1;
    memory.size = info->size;

    return memory;
}

void TestAlloc_free(Allocator *const base, const FreeInfo *const info) {
    TestAlloc *const self = (TestAlloc*)base;

    TestAllocHeader *const target = (TestAllocHeader*)info->memory.data - 1;

    TestAllocHeader** node = &self->head;
    while (*node != NULL) {
        if (*node == target) {
            *node = target->next;
            StdAlloc_free(NULL, &(FreeInfo) {
                .memory = {
                    .data = target,
                    .size = target->size,
                },
            });
            return;
        }

        node = &(*node)->next;
    }

    fprintf(
        stderr,
        "[TestAlloc] Double free detected: %s (line %zu, address: %p)\n",
        info->debugInfo.file,
        info->debugInfo.line,
        info->memory.data
    );
}
