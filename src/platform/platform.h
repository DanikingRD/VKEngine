#ifndef PLATFORM_H
#define PLATFORM_H

#ifndef PLATFORM_DEFINES_H
#define PLATFORM_DEFINES_H

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_MACOS
#endif

#if defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX
#endif

#endif

typedef enum WriteColor {
    PRINT_COLOR_RED = 0,
    PRINT_COLOR_YELLOW = 1,
    PRINT_COLOR_GREEN = 2,
    PRINT_COLOR_BLUE = 3,
    PRINT_COLOR_BLACK = 4,
} WriteColor;

void platform_println(const char* buf, WriteColor color);
void platform_eprintln(const char* buf, WriteColor color);

#endif
