//
// Created by droc101 on 11/5/24.
//

#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

#include "../../defines.h"

#define FLUSH_ON_INFO false
#define FLUSH_ON_DEBUG true
#define FLUSH_ON_WARNING true
#define FLUSH_ON_ERROR true


void LogInternal(const char *type, int color, bool flush, const char *message, ...);

/**
 * Log an info message
 * @param message Format string
 * @param ... Format arguments
 */
#define LogInfo(message, ...) LogInternal("INFO", 37, FLUSH_ON_INFO, message __VA_OPT__(, __VA_ARGS__))

#ifndef NDEBUG
/**
 * Log an info message, but only in debug builds
 * @param message Format string
 * @param ... Format arguments
 */
#define LogDebug(message, ...) LogInternal("DEBUG", 37, FLUSH_ON_DEBUG, message __VA_OPT__(, __VA_ARGS__))
#endif

/**
 * Log a warning message
 * @param message Format string
 * @param ... Format arguments
 */
#define LogWarning(message, ...) LogInternal("INFO", 33, FLUSH_ON_WARNING, message __VA_OPT__(, __VA_ARGS__))

/**
 * Log an error message
 * @param message Format string
 * @param ... Format arguments
 */
#define LogError(message, ...) LogInternal("ERROR", 31, FLUSH_ON_ERROR, message __VA_OPT__(, __VA_ARGS__))

#endif //GAME_LOGGING_H
