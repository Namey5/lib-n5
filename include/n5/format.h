#ifndef __N5_FORMAT_H__
#define __N5_FORMAT_H__

#include "n5/string.h"

typedef struct FormatArg FormatArg;
typedef Slice(const FormatArg) FormatArgs;

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

static inline FormatArg FormatArg_str(cstr val) { return (FormatArg) { .type = fmt_str, .str = val }; }
static inline FormatArg FormatArg_char(char val) { return (FormatArg) { .type = fmt_ch, .ch = val }; }
static inline FormatArg FormatArg_i64(int64_t val) { return (FormatArg) { .type = fmt_i64, .i64 = val }; }
static inline FormatArg FormatArg_u64(uint64_t val) { return (FormatArg) { .type = fmt_u64, .u64 = val }; }
static inline FormatArg FormatArg_f64(double val) { return (FormatArg) { .type = fmt_f64, .f64 = val }; }
static inline FormatArg FormatArg_bool(bool val) { return (FormatArg) { .type = fmt_boolean, .boolean = val }; }

#define String_format(self, format, ...) String_format_raw( \
    (self), \
    (format), \
    (FormatArgs)Slice_fromArray(((FormatArg[]) { __VA_ARGS__ })) \
)

bool String_format_raw(String* self, cstr format, FormatArgs args);

#endif // __N5_FORMAT_H__
