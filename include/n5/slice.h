#ifndef __N5_SLICE_H__
#define __N5_SLICE_H__

#include <stdbool.h>

#include "n5/utils.h"

#ifndef assert
#define assert(condition) ((void)0)
#endif

#define Slice(T) struct { T* data; size_t size; }

#define Slice_start(self) (self).data
#define Slice_end(self) ((self).data + (self).size)
#define Slice_rawSize(self) ((self).size * sizeof((self).data[0]))

#define Slice_from(ptr, len) { .data = (ptr), .size = (len) }
#define Slice_fromRange(start, end) Slice_from((start), (end) - (start))
#define Slice_fromArray(arr) Slice_from((arr), n5_arraySize(arr))

#define Slice_slice(self, offset, len) Slice_from(( \
    assert((offset) >= 0), \
    assert((self).size >= ((len) + (offset))), \
    (self).data + (offset)), \
    (len) \
)

#define Slice_copyTo(dest, src) ( \
    assert((dest).data != NULL), \
    assert((src).data != NULL), \
    assert((dest).size >= (src).size), \
    memcpy((dest).data, (src).data, Slice_rawSize(src)) \
)

#define Slice_compare(a, b) ( \
    ((a).size == (b).size) \
        ? memcmp((a).data, (b).data, Slice_rawSize(a)) \
        : ((int)(a).size - (int)(b).size) \
)

#endif // __N5_SLICE_H__
