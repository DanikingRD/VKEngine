#ifndef LOG_H
#define LOG_H

#include "types.h"

typedef enum LogLevel {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
} LogLevel;

void logger_create(void);
void logger_destroy(void);
void logger_log(LogLevel level, const char* file, u32 number, const char* message, ...);

#define ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__);
#define WARN(...) logger_log(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__);
#define DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__);
#define TRACE(...) logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__);

#endif
