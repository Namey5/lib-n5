#ifndef __N5_STRING_H__
#define __N5_STRING_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "n5/str.h"

typedef struct IAllocator IAllocator;
typedef const IAllocator* Allocator;

typedef struct String String;

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

bool String_append_char(String* self, char character);
bool String_append_str(String* self, cstr other);
bool String_append_u64(String* self, uint64_t value, bool hex);
bool String_append_i64(String* self, int64_t value, bool hex);
bool String_append_f64(String* self, double value);
bool String_append_bool(String* self, bool value);

#endif // __N5_STRING_H__
