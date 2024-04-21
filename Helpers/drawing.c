//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "SDL.h"
#include "drawing.h"
#include <stdio.h>

SDL_Renderer *renderer;

void SetRenderer(SDL_Renderer *r) {
    renderer = r;
}

SDL_Renderer *GetRenderer() {
    return renderer;
}

void draw_rect(int x, int y, int w, int h) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderFillRect(renderer, &rect);
}

// Set the SDL color from an ARGB uint32
void setColorUint(uint color) {
    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF, (color >> 24) & 0xFF);
}

SDL_Texture* ToSDLTexture(const unsigned char* imageData) {
    uint *textureDataUint = (uint*)imageData;
    uint totalLength = textureDataUint[0];
    uint width = textureDataUint[1];
    uint height = textureDataUint[2];
    const unsigned char* pixelData = imageData + (sizeof(uint) * 4); // Skip the first 4 bytes

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)pixelData, width, height, 32, width * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface as it's not needed anymore
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }

    return texture;
}

SDL_Point SDL_TextureSize(SDL_Texture *texture) {
    SDL_Point size;
    SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
    return size;
}

void DrawTextureColumn(SDL_Texture* texture, int sx, int dx, int dy, int dh) {
    SDL_Rect destRect;
    destRect.x = dx;
    destRect.y = dy;
    destRect.w = 1;
    destRect.h = dh;
    SDL_Rect srcRect;
    srcRect.x = sx;
    srcRect.y = 0;
    srcRect.w = 1;
    srcRect.h = SDL_TextureSize(texture).y;
    SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
}
