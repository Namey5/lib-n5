#include "n5/format.h"

#include <assert.h>
#include <stdio.h>

bool String_format_raw(String *const self, const cstr format, const FormatArgs args) {
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

    int32_t currentArg = 0;

    for (const char* fmt = formatStart; fmt < formatEnd; ++fmt) {
        if (*fmt == '{') {
            if (++fmt >= formatEnd) {
                fprintf(stderr, "[String_format] error: incomplete format specifier.\n");
                goto error;
            }

            if (*fmt != '{') {
                const char* end = fmt;
                while (end < formatEnd && *end != '}') {
                    ++end;
                    if (end >= formatEnd) {
                        fprintf(stderr, "[String_format] error: incomplete format specifier.\n");
                        goto error;
                    }
                }

                const cstr specifier = cstr_slice(format, (size_t)(fmt - formatStart), (size_t)(end - fmt));

                uint64_t argIndex;
                if (specifier.size > 0) {
                    if (!str_tryParse_u64(specifier, &argIndex)) {
                        fprintf(stderr, "[String_format] error: failed to parse format specifier.\n");
                        goto error;
                    }

                    if (currentArg > 0) {
                        fprintf(
                            stderr,
                            "[String_format] error: automatic and explicit "
                            "format specifiers may not be used together.\n"
                        );
                        goto error;
                    }

                    currentArg = -1;
                } else {
                    if (currentArg < 0) {
                        fprintf(
                            stderr,
                            "[String_format] error: automatic and explicit "
                            "format specifiers may not be used together.\n"
                        );
                        goto error;
                    }

                    argIndex = currentArg++;
                }

                if (argIndex >= args.size) {
                    fprintf(stderr, "[String_format] error: format specifier was outside args.size.\n");
                    goto error;
                }

                const FormatArg *const arg = &args.data[argIndex];
                switch (arg->type) {
                    case fmt_str: {
                        if (!String_append_str(self, arg->str)) {
                            fprintf(stderr, "[String_format] error: failed to append str.\n");
                            goto error;
                        }
                    } break;

                    case fmt_ch: {
                        if (!String_append_char(self, arg->ch)) {
                            fprintf(stderr, "[String_format] error: failed to append char.\n");
                            goto error;
                        }
                    } break;

                    case fmt_u64: {
                        if (!String_append_u64(self, arg->u64, false)) {
                            fprintf(stderr, "[String_format] error: failed to append u32.\n");
                            goto error;
                        }
                    } break;

                    case fmt_i64: {
                        if (!String_append_i64(self, arg->i64, false)) {
                            fprintf(stderr, "[String_format] error: failed to append i64.\n");
                            goto error;
                        }
                    } break;

                    case fmt_f64: {
                        if (!String_append_f64(self, arg->f64)) {
                            fprintf(stderr, "[String_format] error: failed to append f64.\n");
                            goto error;
                        }
                    } break;

                    case fmt_boolean: {
                        if (!String_append_bool(self, arg->boolean)) {
                            fprintf(stderr, "[String_format] error: failed to append bool.\n");
                            goto error;
                        }
                    } break;

                    default: {
                        fprintf(stderr, "[String_format] error: unrecognised argument type.\n");
                        goto error;
                    } break;
                }

                fmt += specifier.size;
                continue;
            }
        } else if (*fmt == '}') {
            if (++fmt >= formatEnd || *fmt != '}') {
                fprintf(stderr, "[String_format] error: encountered unescaped '}'.\n");
                goto error;
            }
        }

        if (self->str.size >= self->capacity && !String_grow(self, self->str.size + 1)) {
            fprintf(stderr, "[String_format] error: string allocation failed.\n");
            goto error;
        }

        self->str.data[self->str.size++] = *fmt;
    }

    self->str.data[self->str.size] = '\0';

    return true;

error:
    self->str.size = 0;
    self->str.data[0] = '\0';
    return false;
}
