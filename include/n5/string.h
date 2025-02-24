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
#define cstr_cast(string) ((cstr)Slice_from((string).data, (string).size))

#define str_slice(string, offset, len) ((str)Slice_slice((string), (offset), (len)))
#define cstr_slice(string, offset, len) ((cstr)Slice_slice((string), (offset), (len)))

void str_reverse(str self);

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

bool String_format(String* self, cstr format, ...);
bool String_append_char(String* self, char character);
bool String_append_str(String* self, cstr other);
bool String_append_u64(String* self, uint64_t value, bool hex);
bool String_append_i64(String* self, int64_t value, bool hex);
bool String_append_f64(String* self, double value);
bool String_append_bool(String* self, bool value);

#endif // __N5_STRING_H__
