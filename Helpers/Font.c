//
// Created by droc101 on 4/21/2024.
//

#include "Font.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "MathEx.h"
#include "SDL.h"
#include "Drawing.h"
#include "CommonAssets.h"
#include "GL/glHelper.h"
#include "../Assets/Assets.h"

const char fontChars[] = "abcdefghijklmnopqrstuvwxyz0123456789.:-,/\\|[]{}();'\"<>`~!@#$%^*_=+?";

int findChar(char target) {
    int i = 0;
    while (fontChars[i] != 0) {
        if (fontChars[i] == target) {
            return i;
        }
        i++;
    }
    return -1;  // Character not found
}

void FontDrawChar(Vector2 pos, char c, uint size, bool small, uint color) {
    if (c == '?') printf("%c,%d,%d\n", c, findChar(c), findChar(tolower(c)));
    int index = findChar(tolower(c));
    if (index == -1) {
        index = findChar('U');
    }
    SDL_Rect srcRect;
    srcRect.x = index * (small ? SMALL_FONT_CHAR_WIDTH : FONT_CHAR_WIDTH);
    srcRect.y = 0;
    srcRect.w = small ? SMALL_FONT_CHAR_WIDTH : FONT_CHAR_WIDTH;
    srcRect.h = FONT_CHAR_HEIGHT;
    SDL_Rect dstRect;
    dstRect.x = pos.x;
    dstRect.y = pos.y;
    dstRect.w = small ? size * 0.75 : size;
    dstRect.h = size;

    GL_DrawTextureRegionMod(v2(dstRect.x, dstRect.y), v2(dstRect.w, dstRect.h), small ? gztex_interface_small_fonts : gztex_interface_font,
                            v2(srcRect.x, srcRect.y),
                            v2(srcRect.w, srcRect.h), color);
}

Vector2 FontDrawString(Vector2 pos, char* str, uint size, uint color, bool small) {
    //SDL_SetTextureColorMod(small ? smallFontTexture : fontTexture, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    int x = pos.x;
    int y = pos.y;
    int i = 0;
    int sizeX = small ? size * 0.75 : size;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            i++;
            x += sizeX;
        } else if (str[i] == '\n') {
            i++;
            x = pos.x;
            y += size;
        }
        FontDrawChar(v2(x, y), str[i], size, small, color);
        x += sizeX;
        i++;
    }
    return v2(x + sizeX, y + size); // Return the bottom right corner of the text
}

Vector2 MeasureText(char* str, uint size, bool small) {
    int textWidth = 0;
    int textHeight = size;
    int tempWidth = 0;
    int sizeX = small ? size * 0.75 : size;
    for (int j = 0; j < strlen(str); j++) {
        tempWidth += sizeX;
        if (str[j] == '\n') {
            tempWidth -= sizeX;
            textWidth = max(textWidth, tempWidth);
            tempWidth = 0;
            textHeight += size;
        }
    }

    textWidth = max(textWidth, tempWidth);

    return v2(textWidth, textHeight);
}

int StringLineCount(char *str) {
    int count = 1;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '\n') {
            count++;
        }
    }
    return count;
}

int MeasureLine(char *str, int line) {
    int i = 0;
    int count = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            count++;
        }
        if (count == line) {
            return i;
        }
        i++;
    }
    return i;
}

void TextGetLine(char *str, int line, char *out) {
    int start = MeasureLine(str, line);

    // if the start is a newline, skip it
    if (str[start] == '\n') {
        start++;
    }

    int end = MeasureLine(str, line+1);

    // ensure start is less than end
    if (start > end) {
        out[0] = '\0';
        return;
    }

    strncpy(out, str + start, end - start);
    out[end - start] = '\0';
}

void DrawTextAligned(char* str, uint size, uint color, Vector2 rect_pos, Vector2 rect_size, byte h_align, byte v_align, bool small) {
    int lines = StringLineCount(str);
    Vector2 textSize;
    int x;
    int y = rect_pos.y;
    if (v_align == FONT_VALIGN_MIDDLE) {
        y += (rect_size.y - (lines * size)) / 2;
    } else if (v_align == FONT_VALIGN_BOTTOM) {
        y += rect_size.y - (lines * size);
    }

    for (int i = 0; i < lines; i++) {
        char line[256];
        TextGetLine(str, i, line);
        textSize = MeasureText(line, size, small);
        if (h_align == FONT_HALIGN_CENTER) {
            x = rect_pos.x + (rect_size.x - textSize.x) / 2;
        } else if (h_align == FONT_HALIGN_RIGHT) {
            x = rect_pos.x + rect_size.x - textSize.x;
        } else {
            x = rect_pos.x;
        }
        FontDrawString(v2(x, y), line, size, color, small);
        y += size;
    }

}
