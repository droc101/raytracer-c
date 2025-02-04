//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../../defines.h"

// Font alignment
#define FONT_HALIGN_LEFT 0
#define FONT_HALIGN_CENTER 1
#define FONT_HALIGN_RIGHT 2

#define FONT_VALIGN_TOP 0
#define FONT_VALIGN_MIDDLE 1
#define FONT_VALIGN_BOTTOM 2

/**
 * Draw a string of text to the screen
 * @param pos Top left position of the text
 * @param str String to draw
 * @param size Size of the font
 * @param color Color of the font
 * @param font The font to use
 * @return Bottom right position of the text
 */
Vector2 FontDrawString(const Vector2 pos, const char *str, const uint size, const uint color, const Font *font);

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
Vector2 MeasureText(const char *str, uint size, const Font *font);

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
 * @param font The font to use
 */
void DrawTextAligned(const char *str,
					 const uint size,
					 const uint color,
					 const Vector2 rect_pos,
					 const Vector2 rect_size,
					 const byte h_align,
					 const byte v_align,
					 const Font *font);

#endif //GAME_FONT_H
