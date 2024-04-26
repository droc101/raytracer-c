//
// Created by droc101 on 4/26/2024.
//

#include "DPrint.h"
#include "../Helpers/Font.h"
#include <stdarg.h>
#include <stdio.h>

int DPrintYPos = 10;

void ResetDPrintYPos() {
    DPrintYPos = 10;
}

void DPrint(char *str, uint color) {
    DPrintYPos += FontDrawString((Vector2) {10, DPrintYPos}, str, 16, color).y;
}

void DPrintF(char *str, uint color, bool con, ...) {
    char buffer[256];
    va_list args;
    va_start(args, con);
    vsprintf(buffer, str, args);
    va_end(args);
    DPrint(buffer, color);
    if (con) {
        printf("%s\n", buffer);
    }
}

