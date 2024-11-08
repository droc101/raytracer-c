//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdarg.h>
#include <stdio.h>

void LogInfo(const char *str, ...)
{
    printf("\x1b[37;49m[INFO]  ");
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
    printf("\x1b[39;49m");
#ifdef FLUSH_ON_INFO
    fflush(stdout);
#endif
}

void LogDebug(const char *str, ...)
{
#ifndef NDEBUG
    printf("\x1b[37;49m[DEBUG] ");
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
    printf("\x1b[39;49m");
#ifdef FLUSH_ON_DEBUG
    fflush(stdout);
#endif
#endif
}

void LogWarning(const char *str, ...)
{
    printf("\x1b[33;49m[WARN]  ");
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
    printf("\x1b[39;49m");
#ifdef FLUSH_ON_WARNING
    fflush(stdout);
#endif
}

void LogError(const char *str, ...)
{
    printf("\x1b[31;49m[ERROR] ");
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
    printf("\x1b[39;49m");
#ifdef FLUSH_ON_ERROR
    fflush(stdout);
#endif
}
