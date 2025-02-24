#ifndef __N5_UTILS_H__
#define __N5_UTILS_H__

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define n5_min(a, b) ((a) < (b) ? (a) : (b))
#define n5_max(a, b) ((a) > (b) ? (a) : (b))
#define n5_clamp(x, min, max) n5_min((max), n5_max((min), (x)))

#define n5_arraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

#define n5_align_t(ptr) n5_align(ptr, alignof(*ptr))

static inline void* n5_align(void* ptr, size_t align) {
    return (void*)((((uintptr_t)ptr + align - 1) / align) * align);
}

static inline uint64_t n5_nextPow2(uint64_t x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return ++x;
}

#endif // __N5_UTILS_H__
