#include "n5/string.h"

#include <assert.h>
#include <string.h>

#include "n5/alloc.h"
#include "n5/utils.h"

#define MIN_CAPACITY 4

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
