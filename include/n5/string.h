#ifndef __N5_STRING_H__
#define __N5_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "n5/slice.h"

typedef struct IAllocator IAllocator;
typedef const IAllocator* Allocator;

typedef Slice(char) str;
typedef Slice(const char) cstr;

typedef struct String String;

#define str_local(literal) ((str)Slice_from((char[]){ (literal) }, n5_arraySize(literal) - 1))

#define cstr_literal(literal) ((cstr)Slice_from((const char*)(literal), n5_arraySize(literal) - 1))
#define cstr_cast(string) ((cstr)Slice_from((const char*)(string).data, (string).size))

#define str_slice(string, offset, len) ((str)Slice_slice((string), (offset), (len)))
#define cstr_slice(string, offset, len) ((cstr)Slice_slice((string), (offset), (len)))

struct String {
    Allocator* owner;
    size_t capacity;
    str str;
};

String String_new(Allocator* owner, size_t capacity);
String String_from(Allocator* owner, cstr string);
void String_free(String* self);

bool String_resize(String* self, size_t capacity);
bool String_grow(String* self, size_t minSize);

bool String_concat(String* self, cstr other);

#endif // __N5_STRING_H__
