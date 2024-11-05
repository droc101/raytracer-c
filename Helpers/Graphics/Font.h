//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../../Structs/Vector2.h"

/**
 * Draw a string of text to the screen
 * @param pos Top left position of the text
 * @param str String to draw
 * @param size Size of the font
 * @param color Color of the font
 * @param small Whether to use the small font or not
 * @return Bottom right position of the text
 */
Vector2 FontDrawString(const Vector2 pos, const char *str, const uint size, const uint color, const bool small);

/**
 * Count the number of lines in a string
 * @param str String to count lines of
 * @return Line count
 */
int StringLineCount(const char *str);

/**
 * Measure the size of a string of text
 * @param str String to measure
 * @param size Size of the font
 * @param small Whether to use the small font or not
 * @return Size of the text
 */
Vector2 MeasureText(const char *str, uint size, bool small);

/**
 * Get a line of text from a string
 * @param str String to get line from
 * @param line Line number to get
 * @param out GL_Buffer to store the line in
 */
void TextGetLine(const char *str, int line, char *out);

/**
 * Draw a string of text to the screen with alignment
 * @param str String to draw
 * @param size Font size
 * @param color Font color
 * @param rect_pos Top-left position of the rectangle
 * @param rect_size Size of the rectangle
 * @param h_align Horizontal alignment of text within the rectangle
 * @param v_align Vertical alignment of text within the rectangle
 * @param small Whether to use the small font or not
 */
void DrawTextAligned(char *str, uint size, uint color, Vector2 rect_pos, Vector2 rect_size, byte h_align, byte v_align,
                     bool small);

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
