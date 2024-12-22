//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_ERROR_H
#define GAME_ERROR_H

#include "../../config.h"

#if defined BUILDSTYLE_RELEASE || defined ERROR_TRACE_IN_RELEASE
#define Error(error) Error_Internal(error, __FILE_NAME__, __LINE__, __func__)
#else
#define Error(error) Error_Internal(error, "none", 0, "none")
#endif

#define chk_malloc(ptr) \
	if ((ptr) == NULL) _alloc_failure();

void _alloc_failure();

/**
 * Internal error handler
 * @param error Error message
 * @param file File name
 * @param line Line number
 * @param function Function name
 * @warning Do not use this function directly, use the @c Error macro instead
 */
_Noreturn void Error_Internal(char *error, const char *file, int line, const char *function);

/**
 * Friendly error handler
 * @param title Friendly title
 * @param description Friendly description
 */
_Noreturn void FriendlyError(const char *title, const char *description);

/**
 * Ask the user if they want to restart the program (or continue running)
 * @param title The title of the prompt
 * @param description The description/message of the prompt
 * @param yesBtn The text of the yes/restart button
 * @param noBtn The text of the no/continue button
 */
void PromptRelaunch(const char *title, const char *description, const char *yesBtn, const char *noBtn);

/**
 * Shows an error message saying that vk/gl failed to initialize and offers to switch to the other or exit
 */
_Noreturn void RenderInitError();

/**
 * Sets the signal handler to catch @c SIGSEGV and @c SIGFPE
 * @note This intentionally only functions in release mode
 */
void ErrorHandlerInit();

#endif //GAME_ERROR_H
