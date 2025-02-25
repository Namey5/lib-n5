#ifndef __N5_STR_H__
#define __N5_STR_H__

#include <stdbool.h>
#include <stdint.h>

#include "n5/slice.h"

typedef Slice(char) str;
typedef Slice(const char) cstr;

#define str_local(literal) ((str)Slice_from((char[]){ (literal) }, n5_arraySize(literal) - 1))

#define cstr_literal(literal) ((cstr)Slice_from((const char*)(literal), n5_arraySize(literal) - 1))
#define cstr_cast(string) ((cstr)Slice_from((string).data, (string).size))

#define str_slice(string, offset, len) ((str)Slice_slice((string), (offset), (len)))
#define cstr_slice(string, offset, len) ((cstr)Slice_slice((string), (offset), (len)))

void str_reverse(str self);

bool str_tryParse_u64(cstr self, uint64_t* val);
bool str_tryParse_i64(cstr self, int64_t* val);

#endif // __N5_STR_H__
