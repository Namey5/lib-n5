#ifndef __N5_ALLOC_H__
#define __N5_ALLOC_H__

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef assert
#define assert(condition) ((void)0)
#endif

typedef struct IAllocator IAllocator;
typedef const IAllocator* Allocator;

typedef struct Block Block;
typedef struct DebugInfo DebugInfo;
typedef struct AllocInfo AllocInfo;
typedef struct FreeInfo FreeInfo;

typedef struct Arena Arena;

typedef struct TestAlloc TestAlloc;
typedef struct TestAllocHeader TestAllocHeader;

struct IAllocator {
    Block (*alloc)(Allocator* self, const AllocInfo* info);
    void (*free)(Allocator* self, const FreeInfo* info);
};

struct Block {
    void* data;
    size_t size;
};

struct DebugInfo {
    const char* file;
    size_t line;
};

struct AllocInfo {
    size_t size;
    size_t align;
    DebugInfo debugInfo;
};

struct FreeInfo {
    Block memory;
    DebugInfo debugInfo;
};

#define Allocator_alloc(self, type, count) ( \
    assert((self) != NULL), \
    assert(*(self) != NULL), \
    assert((*(self))->alloc != NULL), \
    assert((count) > 0), \
    (*(self))->alloc((self), &(AllocInfo) { \
        .size = sizeof(type) * (count), \
        .align = alignof(type), \
        .debugInfo = { .file = __FILE__, .line = __LINE__ } \
    }) \
)

#define Allocator_free(self, mem) ( \
    assert((self) != NULL), \
    assert((*(self)) != NULL), \
    assert((*(self))->free != NULL), \
    assert((mem).data != NULL), \
    (*(self))->free((self), &(FreeInfo) { \
        .memory = (mem), \
        .debugInfo = { .file = __FILE__, .line = __LINE__ } \
    }) \
)

#define Allocator_createItem(self, type) Allocator_alloc((self), type, 1).data

#define Allocator_destroyItem(self, item) Allocator_free((self), ((Block) { \
    .data = (item), \
    .size = sizeof(*(item)), \
}))

#define Allocator_createItems(self, type, count) { \
    .data = Allocator_alloc((self), type, (count)).data, \
    .size = (count), \
}

#define Allocator_destroyItems(self, items) Allocator_free((self), ((Block) { \
    .data = (items).data, \
    .size = (items).size * sizeof((items).data[0]), \
}))

Allocator StdAlloc_init(void);
Block StdAlloc_alloc(Allocator* self, const AllocInfo* info);
void StdAlloc_free(Allocator* self, const FreeInfo* info);

struct Arena {
    Allocator base;
    Allocator* owner;
    Block pool;
    size_t offset;
};

bool Arena_init(Arena* self, Allocator* owner, size_t size);
void Arena_deinit(Arena* self);
void Arena_reset(Arena* self);
Block Arena_alloc(Allocator* self, const AllocInfo* info);
void Arena_free(Allocator* self, const FreeInfo* info);

struct TestAlloc {
    Allocator base;
    TestAllocHeader* head;
};

struct TestAllocHeader {
    DebugInfo debugInfo;
    size_t size;
    TestAllocHeader* next;
};

TestAlloc TestAlloc_init(void);
void TestAlloc_deinit(TestAlloc* self);
Block TestAlloc_alloc(Allocator* self, const AllocInfo* info);
void TestAlloc_free(Allocator* self, const FreeInfo* info);

#endif // __N5_ALLOC_H__
