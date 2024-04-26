//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_DPRINT_H
#define GAME_DPRINT_H

#include "../defines.h"

// Called to reset the Y position of the debug print (use only once at the start of the frame)
void ResetDPrintYPos();

// Prints a string to the screen and optionally to the console (stdout) (uses printf style formatting)
void DPrintF(char *str, uint color, bool con, ...);

#endif //GAME_DPRINT_H
