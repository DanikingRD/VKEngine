#include "platform.h"

#ifdef PLATFORM_MACOS
#include <stdio.h>

void platform_println(const char* buf, WriteColor color) {
    // 30 = black, 31 = red, 32 = green, 33 = yellow = 34 = blue
    const char* colors[] = {"1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colors[color], buf);
}

void platform_eprintln(const char* buf, WriteColor color) {
    // 30 = black, 31 = red, 32 = green, 33 = yellow = 34 = blue
    const char* colors[] = {"1;31", "1;33", "1;32", "1;34", "1;30"};
    fprintf(stderr, "\033[%sm%s\033[0m", colors[color], buf);
}
#endif
