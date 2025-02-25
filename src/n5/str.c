#include "n5/str.h"

#include <assert.h>

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

bool str_tryParse_u64(const cstr self, uint64_t *const val) {
    assert(self.data != NULL);
    assert(val != NULL);

    *val = 0;
    for (size_t i = 0; i < self.size; ++i) {
        if (self.data[i] < '0' || self.data[i] > '9' || *val > (UINT64_MAX / 10)) {
            return false;
        }

        *val *= 10;
        *val += (self.data[i] - '0');
    }
    return true;
}

bool str_tryParse_i64(cstr self, int64_t *const val) {
    assert(self.data != NULL);
    assert(val != NULL);

    const bool negate = (self.data[0] == '-');
    if (negate) {
        ++self.data;
        --self.size;
    }

    uint64_t u64;
    const bool result = str_tryParse_u64(self, &u64);
    if (result && u64 <= INT64_MAX) {
        *val = (int64_t)u64 * (negate ? -1 : 1);
        return true;
    }
    return false;
}
