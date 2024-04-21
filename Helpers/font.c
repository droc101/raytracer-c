//
// Created by droc101 on 4/21/2024.
//

#include "font.h"
#include "SDL.h"
#include "drawing.h"
#include "../assets/assets.h"
#include "../Structs/Vector2.h"
#include <ctype.h>

SDL_Texture *fontTexture;

const char fontChars[] = "abcdefghijklmnopqrstuvwxyz0123456789.:-";

void FontInit() {
    fontTexture = ToSDLTexture(tex_interface_font);
}

int findChar(char target) {
    int i = 0;
    while (fontChars[i] != '\0') {
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

void FontDrawString(Vector2 pos, char* str, uint size) {
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
}
