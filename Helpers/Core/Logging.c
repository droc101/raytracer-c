//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdio.h>

#include "Error.h"

/// The length of the longest value passed to the type argument of the LogInternal function, plus one
#define longestType 6

FILE *LogFile = NULL;

void LogInit()
{
	char *folderPath = SDL_GetPrefPath(APPDATA_ORG_NAME, APPDATA_APP_NAME);
	const char *fileName = "game.log";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	chk_malloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);

	SDL_free(folderPath);

	LogFile = fopen(filePath, "w");
	free(filePath);
	if (LogFile == NULL)
	{
		Error("Failed to open log file");
	}
}

void LogDestroy()
{
	if (LogFile != NULL)
	{
		fclose(LogFile);
	}
}

void LogInternal(const char *type, const int color, const bool flush, const char *message, va_list args)
{
	va_list args2 = {0};
	va_copy(args2, args);

	char buf[10 + longestType];
	sprintf(buf, "\x1b[%02d;49m[%s]", color, type);
	printf("%-1" TO_STR(longestType) "s", buf);
	vprintf(message, args);
	printf("\x1b[0m");
	if (flush)
	{
		fflush(stdout);
	}
	if (LogFile != NULL)
	{
		fprintf(LogFile, "[%s] ", type);
		vfprintf(LogFile, message, args2);
		if (flush)
		{
			fflush(LogFile);
		}
	}
}

void LogInfo(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	va_end(args);
	LogInternal("INFO", 37, FLUSH_ON_INFO, message, args);
}

void LogDebug(const char *message, ...)
{
#ifdef BUILDSTYLE_DEBUG
	va_list args;
	va_start(args, message);
	va_end(args);
	LogInternal("DEBUG", 37, FLUSH_ON_DEBUG, message, args);
#endif
}

void LogWarning(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	va_end(args);
	LogInternal("INFO", 33, FLUSH_ON_WARNING, message, args);
}

void LogError(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	va_end(args);
	LogInternal("ERROR", 31, FLUSH_ON_ERROR, message, args);
}
