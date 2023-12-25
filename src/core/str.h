#ifndef STR_H
#define STR_H

#include "types.h"

u64 str_length(const char* str);
bool str_equals(const char* str_a, const char* str_b);
i32 str_format(char* buffer, u32 size, const char* format, ...);

#endif
