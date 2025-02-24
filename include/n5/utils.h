#ifndef __N5_UTILS_H__
#define __N5_UTILS_H__

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define n5_arraySize(arr) (sizeof(arr) / sizeof((arr)[0]))

#define n5_align_t(ptr) n5_align(ptr, alignof(*ptr))

static inline void* n5_align(void* ptr, size_t align) {
    return (void*)((((uintptr_t)ptr + align - 1) / align) * align);
}

#endif // __N5_UTILS_H__
