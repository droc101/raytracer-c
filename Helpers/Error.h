//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_ERROR_H
#define GAME_ERROR_H

#include "../config.h"

#if !defined NDEBUG || defined ERROR_TRACE_IN_RELEASE
#define Error(error) _Error_Internal(error, __FILE_NAME__, __LINE__, __func__)
#else
#define Error(error) _Error_Internal(error, "none", 0, "none")
#endif

/**
 * Internal error handler
 * @param error Error message
 * @param file File name
 * @param line Line number
 * @param function Function name
 * @warning Do not use this function directly, use the Error macro instead
 */
_Noreturn void _Error_Internal(char* error, const char* file, int line, const char* function);

/**
 * Friendly error handler
 * @param title Friendly title
 * @param description Friendly description
 */
_Noreturn void FriendlyError(char* title, char* description);

/**
 * Sets the signal handler to catch @c SIGSEGV and @c SIGFPE
 * @note This intentionally only functions in release mode
 */
void SetSignalHandler();

#endif //GAME_ERROR_H
