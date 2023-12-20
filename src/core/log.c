#include "log.h"
#include "platform/platform.h"
#include <stdarg.h>
#include <stdio.h>

void logger_create(void) {}
void logger_destroy(void) {}

void logger_log(LogLevel level, const char* file, u32 number, const char* message, ...) {
    const char* LOG_LEVELS[5] = {"ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
    char fmt[4096];
    va_list args;
    va_start(args, message);
    vsnprintf(fmt, 4096, message, args);
    va_end(args);

    bool err = level <= LOG_LEVEL_ERROR;
    char buf[4096];
    // format: [LOG_LEVEL]: file:line: message
    snprintf(buf, 4096, "[%s %s:%d]: %s\n", LOG_LEVELS[level], file, number, fmt);
    if (err) {
        platform_eprintln(buf, (WriteColor)level);
    } else {
        platform_println(buf, (WriteColor)level);
    }
}
