//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../Structs/Vector2.h"

// Draw a string to the screen
Vector2 FontDrawString(Vector2 pos, char* str, uint size, uint color, bool small);

int StringLineCount(char *str);

Vector2 MeasureText(char* str, uint size, bool small);

void TextGetLine(char *str, int line, char *out);

void DrawTextAligned(char* str, uint size, uint color, Vector2 rect_pos, Vector2 rect_size, byte h_align, byte v_align, bool small);

// Texture sizes of the font
#define FONT_CHAR_WIDTH 16
#define FONT_CHAR_HEIGHT 16

#define SMALL_FONT_CHAR_WIDTH 12

#define FONT_HALIGN_LEFT 0
#define FONT_HALIGN_CENTER 1
#define FONT_HALIGN_RIGHT 2

#define FONT_VALIGN_TOP 0
#define FONT_VALIGN_MIDDLE 1
#define FONT_VALIGN_BOTTOM 2

#endif //GAME_FONT_H
