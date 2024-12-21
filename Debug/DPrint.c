//
// Created by droc101 on 4/26/2024.
//

#include "DPrint.h"
#include <stdio.h>
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/Font.h"

int DPrintYPos = 10;

void ResetDPrintYPos()
{
	DPrintYPos = 10;
}

void DPrint(const char *str, const uint color)
{
#ifdef ENABLE_DEBUG_PRINT
	FontDrawString((Vector2){12, DPrintYPos + 2}, str, 16, 0xFF000000, true);
	DPrintYPos += FontDrawString((Vector2){10, DPrintYPos}, str, 16, color, true).y - DPrintYPos + 8;
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
