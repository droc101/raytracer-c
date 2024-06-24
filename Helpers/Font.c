//
// Created by droc101 on 4/21/2024.
//

#include "Font.h"
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "MathEx.h"
#include "SDL.h"
#include "Drawing.h"
#include "../Assets/Assets.h"
#include "../Assets/AssetReader.h"

SDL_Texture *fontTexture;

const char fontChars[] = "abcdefghijklmnopqrstuvwxyz0123456789.:-,/\\|[]{}();'\"<>`~!@#$%^&*()_=+?";

void FontInit() {
    fontTexture = ToSDLTexture((const unsigned char *) gztex_interface_font, FILTER_LINEAR);
}

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

void FontDrawChar(Vector2 pos, char c, uint size) {
    int index = findChar(tolower(c));
    if (index == -1) {
        index = findChar('U');
    }
    SDL_Rect srcRect;
    srcRect.x = index * FONT_CHAR_WIDTH;
    srcRect.y = 0;
    srcRect.w = FONT_CHAR_WIDTH;
    srcRect.h = FONT_CHAR_HEIGHT;
    SDL_Rect dstRect;
    dstRect.x = pos.x;
    dstRect.y = pos.y;
    dstRect.w = size;
    dstRect.h = size;
    SDL_RenderCopy(GetRenderer(), fontTexture, &srcRect, &dstRect);
}

Vector2 FontDrawString(Vector2 pos, char* str, uint size, uint color) {
    SDL_SetTextureColorMod(fontTexture, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    int x = pos.x;
    int y = pos.y;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            i++;
            x += size;
        } else if (str[i] == '\n') {
            i++;
            x = pos.x;
            y += size;
        }
        FontDrawChar(vec2(x, y), str[i], size);
        x += size;
        i++;
    }
    return vec2(x+size, y+size); // Return the bottom right corner of the text
}

Vector2 MeasureText(char* str, uint size) {
    int textWidth = 0;
    int textHeight = size;
    int tempWidth = 0;
    for (int j = 0; j < strlen(str); j++) {
        tempWidth += size;
        if (str[j] == '\n') {
            tempWidth -= size;
            textWidth = max(textWidth, tempWidth);
            tempWidth = 0;
            textHeight += size;
        }
    }

    textWidth = max(textWidth, tempWidth);

    return vec2(textWidth, textHeight);
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

void DrawTextAligned(char* str, uint size, uint color, Vector2 rect_pos, Vector2 rect_size, byte h_align, byte v_align) {
    int lines = StringLineCount(str);
    Vector2 textSize = MeasureText(str, size);
    int x = rect_pos.x;
    int y = rect_pos.y;
    if (v_align == FONT_VALIGN_MIDDLE) {
        y += (rect_size.y - (lines * size)) / 2;
    } else if (v_align == FONT_VALIGN_BOTTOM) {
        y += rect_size.y - (lines * size);
    }

    for (int i = 0; i < lines; i++) {
        int lineStart = MeasureLine(str, i);
        int lineEnd = MeasureLine(str, i+1);
        char line[256];
        strncpy(line, str + lineStart, lineEnd - lineStart);
        line[lineEnd - lineStart] = '\0';
        textSize = MeasureText(line, size);
        if (h_align == FONT_HALIGN_CENTER) {
            x = rect_pos.x + (rect_size.x - textSize.x) / 2;
        } else if (h_align == FONT_HALIGN_RIGHT) {
            x = rect_pos.x + rect_size.x - textSize.x;
        } else {
            x = rect_pos.x;
        }
        FontDrawString(vec2(x, y), line, size, color);
//        y += size;
    }

}
