#include "str.h"
#include <string.h>

u64 str_length(const char* str) { return strlen(str); }
bool str_equals(const char* a, const char* b) { return strcmp(a, b) == 0; }
