//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../Structs/Vector2.h"

// Set up the font system
void FontInit();

// Draw a string to the screen
void FontDrawString(Vector2 pos, char* str, uint size, uint color);

// Texture sizes of the font
#define FONT_CHAR_WIDTH 16
#define FONT_CHAR_HEIGHT 16

#endif //GAME_FONT_H
