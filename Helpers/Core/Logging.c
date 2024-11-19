//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdio.h>

/// The length of the longest value passed to the type argument of the LogInternal function, plus one
#define longestType 6

void LogInternal(const char *type, const int color, const bool flush, const char *message, const va_list args)
{
    char buf[10 + longestType];
    sprintf(buf, "\x1b[%02d;49m[%s]", color, type);
    printf("%-1"TO_STR(longestType)"s", buf);
    vprintf(message, args);
    printf("\x1b[0m");
    if (flush) fflush(stdout);
}

void LogInfo(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    LogInternal("INFO", 37, FLUSH_ON_INFO, message, args);
    va_end(args);
}

void LogDebug(const char *message, ...)
{
#ifndef NDEBUG
    va_list args;
    va_start(args, message);
    LogInternal("DEBUG", 37, FLUSH_ON_DEBUG, message, args);
    va_end(args);
#endif
}

void LogWarning(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    LogInternal("INFO", 33, FLUSH_ON_WARNING, message, args);
    va_end(args);
}

void LogError(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    LogInternal("ERROR", 31, FLUSH_ON_ERROR, message, args);
    va_end(args);
}
