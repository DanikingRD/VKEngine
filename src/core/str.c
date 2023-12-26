#include "str.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
u64 str_length(const char* str) { return strlen(str); }
bool str_equals(const char* a, const char* b) { return strcmp(a, b) == 0; }

i32 str_format(char* buffer, u32 size, const char* format, ...) {
    if (!buffer)
        return 0;
    va_list args;
    va_start(args, format);
    i32 result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}
