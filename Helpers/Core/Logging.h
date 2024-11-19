//
// Created by droc101 on 11/5/24.
//

#ifndef GAME_LOGGING_H
#define GAME_LOGGING_H

//#define FLUSH_ON_INFO
#define FLUSH_ON_DEBUG
#define FLUSH_ON_WARNING
#define FLUSH_ON_ERROR

/**
 * Log an info message
 * @param message Format string
 * @param ... Format arguments
 */
void LogInfo(const char *message, ...);

/**
 * Log an info message, but only in debug builds
 * @param message Format string
 * @param ... Format arguments
 */
void LogDebug(const char *message, ...);

/**
 * Log a warning message
 * @param message Format string
 * @param ... Format arguments
 */
void LogWarning(const char *message, ...);

/**
 * Log an error message
 * @param message Format string
 * @param ... Format arguments
 */
void LogError(const char *message, ...);

#endif //GAME_LOGGING_H
