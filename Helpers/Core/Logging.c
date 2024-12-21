//
// Created by droc101 on 11/5/24.
//

#include "Logging.h"
#include <stdio.h>

/// The length of the longest value passed to the type argument of the LogInternal function (including the null) plus 7
#define bufferLength 14

void LogInternal(const char *type, const int color, const bool flush, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	char buf[bufferLength];
	sprintf(buf, "\x1b[%02dm[%s]", color, type);
	printf("%-" TO_STR(bufferLength) "s", buf);
	vprintf(message, args);
	printf("\x1b[0m");
	if (flush)
	{
		fflush(stdout);
	}
	va_end(args);
}
