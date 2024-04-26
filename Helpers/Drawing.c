//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "../defines.h"
#include "SDL.h"
#include "Drawing.h"
#include "Error.h"

SDL_Renderer *renderer;
SDL_Window *window;

void SetRenderer(SDL_Renderer *r) {
    renderer = r;
}

SDL_Renderer *GetRenderer() {
    return renderer;
}

void SetWindow(SDL_Window *w) {
    window = w;
}

SDL_Window *GetWindow() {
    return window;
}

int WindowWidth() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return w;
}

int WindowHeight() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return h;
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

byte* getColorUint(uint color) {
    byte *buf = malloc(4);
    buf[0] = (color >> 16) & 0xFF;
    buf[1] = (color >> 8) & 0xFF;
    buf[2] = (color >> 0) & 0xFF;
    buf[3] = (color >> 24) & 0xFF;
    return buf;
}

SDL_Surface* ToSDLSurface(const unsigned char* imageData, char *filterMode) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);
    uint *textureDataUint = (uint*)imageData;
    uint width = textureDataUint[1];
    uint height = textureDataUint[2];
    const unsigned char* pixelData = imageData + (sizeof(uint) * 4); // Skip the first 4 bytes

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)pixelData, width, height, 32, width * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        Error("ToSDLTexture: Failed to create surface");
        return NULL;
    }

    return surface;
}

SDL_Texture* ToSDLTexture(const unsigned char* imageData, char *filterMode) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);

    SDL_Surface* surface = ToSDLSurface(imageData, filterMode);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        Error("ToSDLTexture: Failed to create surface");
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface as it's not needed anymore
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        Error("ToSDLTexture: Failed to create texture");
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

SDL_Texture *GetScreenshot() {
    SDL_Surface *ss = SDL_CreateRGBSurface(0, WindowWidth(), WindowHeight(), 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!ss) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        Error("GetScreenshot: Failed to create surface");
        return NULL;
    }

    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, ss->pixels, ss->pitch);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, ss);
    SDL_FreeSurface(ss);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        Error("GetScreenshot: Failed to create texture");
        return NULL;
    }

    return texture;
}
