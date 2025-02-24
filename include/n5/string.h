#ifndef __N5_STRING_H__
#define __N5_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "n5/slice.h"

typedef Slice(char) str;
typedef Slice(const char) cstr;

#define str_local(literal) ((str)Slice_from((char[]){ (literal) }, n5_arraySize(literal) - 1))

#define cstr_literal(literal) ((cstr)Slice_from((const char*)(literal), n5_arraySize(literal) - 1))
#define cstr_cast(string) ((cstr)Slice_from((const char*)(string).data, (string).size))

#define str_slice(string, offset, len) ((str)Slice_slice((string), (offset), (len)))
#define cstr_slice(string, offset, len) ((cstr)Slice_slice((string), (offset), (len)))

#endif // __N5_STRING_H__
