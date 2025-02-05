//
// Created by droc101 on 4/26/2024.
//

#include "DPrint.h"
#include <stdio.h>

#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/Font.h"

int dprintYPosition = 10;

void ResetDPrintYPos()
{
	dprintYPosition = 10;
}

void DPrint(const char *str, const uint color)
{
#ifdef ENABLE_DEBUG_PRINT
	FontDrawString((Vector2){12, (float)dprintYPosition + 2}, str, 16, 0xFF000000, smallFont);
	dprintYPosition += (int)FontDrawString((Vector2){10, (float)dprintYPosition}, str, 16, color, smallFont).y -
					   dprintYPosition +
					   8;
#endif
}

void DPrintF(const char *str, const uint color, const bool con, ...)
{
#ifdef ENABLE_DEBUG_PRINT
	char buffer[256];
	va_list args;
	va_start(args, con);
	vsprintf(buffer, str, args);
	va_end(args);
	DPrint(buffer, color);
	if (con)
	{
		LogInfo(buffer);
	}
#endif
}
