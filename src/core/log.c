#include "log.h"
#include "platform/fs.h"
#include "platform/platform.h"
#include "str.h"
#include <stdarg.h>
#include <stdio.h>

typedef struct Logger {
    File handle;
} Logger;

static Logger logger;

bool logger_create(void) {
    if (!fs_open("latest.log", OPEN_FILE_MODE_WRITE, &logger.handle)) {
        ERROR("Failed to create/open log file: latest.log");
        return false;
    }
    INFO("Logger initialized");
    return true;
}

void logger_log_file(const char* message) {
    if (logger.handle.handle) {
        u64 len = str_length(message);
        u64 bytes_written = 0;
        if (!fs_write(&logger.handle, len, message, &bytes_written)) {
            ERROR("Failed to write to log file");
        }
    }
}

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

    logger_log_file(buf);
}
