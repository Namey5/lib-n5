#include "n5/string.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "n5/alloc.h"
#include "n5/slice.h"
#include "n5/utils.h"

#define MIN_CAPACITY 4

void str_reverse(const str self) {
    assert(self.data != NULL);
    char* start = self.data;
    char* end = start + (self.size - 1);
    while (start < end) {
        char tmp = *start;
        *(start++) = *end;
        *(end--) = tmp;
    }
}

void String_freeInternal(String *const self) {
    Allocator_free(self->owner, ((Block) {
        .data = self->str.data,
        .size = self->capacity + 1,
    }));
}

String String_new(Allocator *const owner, const size_t capacity) {
    assert(owner != NULL);

    String self = (String) { .owner = owner };
    String_resize(&self, capacity);
    return self;
}

String String_from(Allocator *const owner, const cstr string) {
    assert(owner != NULL);

    String self = String_new(owner, string.size);
    if (string.data != NULL) {
        self.str.size = string.size;
        Slice_copyTo(self.str, string);
        self.str.data[self.str.size] = '\0';
    }
    return self;
}

void String_free(String *const self) {
    assert(self != NULL);
    String_freeInternal(self);
    *self = (String) { 0 };
}

bool String_resize(String *const self, size_t capacity) {
    assert(self != NULL);
    assert(self->owner != NULL);

    capacity = n5_max(capacity, MIN_CAPACITY);
    if (self->capacity == capacity) {
        return true;
    }

    // note: add 1 to capacity here for null terminator.
    //  - need to also do so when we free self->str.
    Block buffer = Allocator_alloc(self->owner, char, capacity + 1);
    if (buffer.data == NULL) {
        return false;
    }

    str newStr = Slice_from(buffer.data, self->str.size);
    if (self->str.data != NULL) {
        Slice_copyTo(newStr, self->str);
        String_freeInternal(self);
    } else {
        newStr.data[0] = '\0';
    }

    self->str = newStr;
    self->capacity = capacity;

    return true;
}

bool String_grow(String *const self, const size_t minSize) {
    assert(self != NULL);

    if (self->capacity >= minSize) {
        return true;
    }

    size_t capacity = self->capacity * 2;
    capacity = (capacity > minSize) ? capacity : minSize;
    return String_resize(self, capacity);
}

bool String_format(String *const self, const cstr format, ...) {
    assert(self != NULL);
    assert(format.data != NULL);

    self->str.size = 0;
    if (!String_grow(self, format.size)) {
        fprintf(stderr, "[String_format] error: string allocation failed.\n");
        return false;
    }

    self->str.data[0] = '\0';

    const char *const formatStart = Slice_start(format);
    const char *const formatEnd = Slice_end(format);

    va_list args;
    va_start(args, format);
    for (const char* fmt = formatStart; fmt < formatEnd; ++fmt) {
        if (*fmt == '{') {
            if (++fmt >= formatEnd) {
                fprintf(stderr, "[String_format] error: incomplete format specifier.\n");
                goto error;
            }

            if (*fmt != '{') {
                const char* end = fmt + 1;
                while (end < formatEnd && *end != '}') {
                    ++end;
                }
                if (*end != '}') {
                    fprintf(stderr, "[String_format] error: incomplete format specifier.\n");
                    goto error;
                }

                const cstr specifier = cstr_slice(format, (size_t)(fmt - formatStart), (size_t)(end - fmt));
                if (Slice_compare(specifier, cstr_literal("str")) == 0) {
                    cstr string = va_arg(args, cstr);
                    if (!String_append_str(self, string)) {
                        fprintf(stderr, "[String_format] error: failed to append str.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("c")) == 0) {
                    char c = va_arg(args, uint32_t) & 0xff;
                    if (!String_append_char(self, c)) {
                        fprintf(stderr, "[String_format] error: failed to append char.\n");
                        goto error;
                    }
                } else if (
                    Slice_compare(specifier, cstr_literal("u8")) == 0
                    || Slice_compare(specifier, cstr_literal("u16")) == 0
                    || Slice_compare(specifier, cstr_literal("u32")) == 0
                ) {
                    uint32_t u32 = va_arg(args, uint32_t);
                    if (!String_append_u64(self, u32, false)) {
                        fprintf(stderr, "[String_format] error: failed to append u32.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("u64")) == 0) {
                    uint64_t u64 = va_arg(args, uint64_t);
                    if (!String_append_u64(self, u64, false)) {
                        fprintf(stderr, "[String_format] error: failed to append u64.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("usize")) == 0) {
                    uintptr_t usize = va_arg(args, uintptr_t);
                    if (!String_append_u64(self, usize, false)) {
                        fprintf(stderr, "[String_format] error: failed to append usize.\n");
                        goto error;
                    }
                } else if (
                    Slice_compare(specifier, cstr_literal("i8")) == 0
                    || Slice_compare(specifier, cstr_literal("i16")) == 0
                    || Slice_compare(specifier, cstr_literal("i32")) == 0
                ) {
                    int32_t i32 = va_arg(args, int32_t);
                    if (!String_append_i64(self, i32, false)) {
                        fprintf(stderr, "[String_format] error: failed to append i32.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("i64")) == 0) {
                    int64_t i64 = va_arg(args, int64_t);
                    if (!String_append_i64(self, i64, false)) {
                        fprintf(stderr, "[String_format] error: failed to append i64.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("isize")) == 0) {
                    intptr_t isize = va_arg(args, intptr_t);
                    if (!String_append_i64(self, isize, false)) {
                        fprintf(stderr, "[String_format] error: failed to append isize.\n");
                        goto error;
                    }
                } else if (
                    Slice_compare(specifier, cstr_literal("f32")) == 0
                    || Slice_compare(specifier, cstr_literal("f64")) == 0
                ) {
                    double f64 = va_arg(args, double);
                    if (!String_append_f64(self, f64)) {
                        fprintf(stderr, "[String_format] error: failed to append f64.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("ptr")) == 0) {
                    void *const ptr = va_arg(args, void*);
                    if (!String_append_u64(self, (uint64_t)ptr, true)) {
                        fprintf(stderr, "[String_format] error: failed to append ptr.\n");
                        goto error;
                    }
                } else if (Slice_compare(specifier, cstr_literal("bool")) == 0) {
                    bool val = (bool)va_arg(args, int32_t);
                    if (!String_append_bool(self, val)) {
                        fprintf(stderr, "[String_format] error: failed to append bool.\n");
                        goto error;
                    }
                } else {
                    fprintf(stderr, "[String_format] error: unknown format specifier.\n");
                    goto error;
                }

                fmt += specifier.size;
                continue;
            }
        }

        if (self->str.size >= self->capacity && !String_grow(self, self->str.size + 1)) {
            fprintf(stderr, "[String_format] error: string allocation failed.\n");
            goto error;
        }

        self->str.data[self->str.size++] = *fmt;
    }
    va_end(args);

    self->str.data[self->str.size] = '\0';

    return true;

error:
    self->str.size = 0;
    self->str.data[0] = '\0';
    return false;
}

bool String_append_char(String *const self, const char character) {
    assert(self != NULL);

    if (!String_grow(self, self->str.size + 1)) {
        return false;
    }

    self->str.data[self->str.size++] = character;
    self->str.data[self->str.size] = '\0';

    return true;
}

bool String_append_str(String *const self, cstr other) {
    assert(self != NULL);

    // edge case: 'other' is a substring of 'self'
    intptr_t localOffset = other.data - self->str.data;
    if ((size_t)localOffset > self->str.size) {
        localOffset = -1;
    }

    const size_t newSize = self->str.size + other.size;
    if (!String_grow(self, newSize)) {
        return false;
    }

    if (localOffset >= 0) {
        assert((localOffset + other.size) <= self->str.size);
        other = (cstr)Slice_from(self->str.data + localOffset, other.size);
    }

    Slice_copyTo((str)Slice_from(Slice_end(self->str), other.size), other);
    self->str.size = newSize;
    self->str.data[self->str.size] = '\0';

    return true;
}

bool String_append_u64(String *const self, uint64_t value, const bool hex) {
    assert(self != NULL);

    const size_t startSize = self->str.size;
    if (hex) {
        for (uint64_t i = n5_max(n5_nextPow2(value) - 1, 0xff); i > 0; i >>= 4) {
            const int8_t x = value & 0x0f;
            const char c = ((x < 0x0a) ? '0' : ('a' - 0x0a)) + x;
            if (!String_append_char(self, c)) {
                goto error;
            }
            value >>= 4;
        }
        if (!String_append_str(self, cstr_literal("x0"))) {
            goto error;
        }
    } else {
        do {
            if (!String_append_char(self, '0' + (value % 10))) {
                goto error;
            }
            value /= 10;
        } while (value != 0);
    }

    str_reverse(str_slice(self->str, startSize, self->str.size - startSize));

    return true;

error:
    self->str.size = startSize;
    self->str.data[startSize] = '\0';
    return false;
}

bool String_append_i64(String *const self, int64_t value, const bool hex) {
    assert(self != NULL);

    if (hex) {
        return String_append_u64(self, *(uint64_t*)&value, true);
    }

    const size_t startSize = self->str.size;

    if (value < 0) {
        if (!String_append_char(self, '-')) {
            goto error;
        }
        value = -value;
    }
    return String_append_u64(self, (uint64_t)value, false);

error:
    self->str.size = startSize;
    self->str.data[startSize] = '\0';
    return false;
}

bool String_append_f64(String *const self, double value) {
    assert(self != NULL);

    const size_t startSize = self->str.size;

    if (value < 0) {
        if (!String_append_char(self, '-')) {
            goto error;
        }
        value = -value;
    }

    if (!String_append_u64(self, (uint64_t)value, false)) {
        goto error;
    }

    if (!String_append_char(self, '.')) {
        goto error;
    }

    value -= (double)(uint64_t)value;
    const int32_t decimalPlaces = 4;
    for (int32_t i = 0; i < decimalPlaces; ++i) {
        value *= 10.0;
    }
    return String_append_u64(self, (uint64_t)value, false);

error:
    self->str.size = startSize;
    self->str.data[startSize] = '\0';
    return false;
}

bool String_append_bool(String *const self, bool value) {
    return String_append_str(self, (value ? cstr_literal("true") : cstr_literal("false")));
}
