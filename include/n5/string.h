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
typedef struct FormatArg FormatArg;
typedef Slice(const FormatArg) FormatArgs;

#define str_local(literal) ((str)Slice_from((char[]){ (literal) }, n5_arraySize(literal) - 1))

#define cstr_literal(literal) ((cstr)Slice_from((const char*)(literal), n5_arraySize(literal) - 1))
#define cstr_cast(string) ((cstr)Slice_from((string).data, (string).size))

#define str_slice(string, offset, len) ((str)Slice_slice((string), (offset), (len)))
#define cstr_slice(string, offset, len) ((cstr)Slice_slice((string), (offset), (len)))

void str_reverse(str self);

bool str_tryParse_u64(cstr self, uint64_t* val);
bool str_tryParse_i64(cstr self, int64_t* val);

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

struct FormatArg {
    enum {
        fmt_str,
        fmt_ch,
        fmt_i64,
        fmt_u64,
        fmt_f64,
        fmt_boolean,
    } type;
    union {
        cstr str;
        char ch;
        int64_t i64;
        uint64_t u64;
        double f64;
        bool boolean;
    };
};

#define FormatArg_from(val) _Generic((val), \
    cstr: FormatArg_str, \
    char: FormatArg_char, \
    int64_t: FormatArg_i64, \
    uint64_t: FormatArg_u64, \
    double: FormatArg_f64, \
    bool: FormatArg_bool \
)(val)

static inline FormatArg FormatArg_str(cstr val) { return ((FormatArg) { .type = fmt_str, .str = val }); }
static inline FormatArg FormatArg_char(char val) { return ((FormatArg) { .type = fmt_ch, .ch = val }); }
static inline FormatArg FormatArg_i64(int64_t val) { return ((FormatArg) { .type = fmt_i64, .i64 = val }); }
static inline FormatArg FormatArg_u64(uint64_t val) { return ((FormatArg) { .type = fmt_u64, .u64 = val }); }
static inline FormatArg FormatArg_f64(double val) { return ((FormatArg) { .type = fmt_f64, .f64 = val }); }
static inline FormatArg FormatArg_bool(bool val) { return ((FormatArg) { .type = fmt_boolean, .boolean = val }); }

#define String_format(self, format, ...) String_format_raw( \
    (self), \
    (format), \
    (FormatArgs)Slice_fromArray(((FormatArg[]) { __VA_ARGS__ })) \
)

bool String_format_raw(String* self, cstr format, FormatArgs args);

#endif // __N5_STRING_H__
