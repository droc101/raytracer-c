//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "SDL.h"
#include "drawing.h"

// Draw an image to screen (from assets.h)
void draw_texture(SDL_Renderer *renderer, const uint tex[], uint sx, uint sy) {
    uint width = tex[1];
    uint height = tex[2];
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // blend using blend
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            uint pixel = tex[4+((y*width)+x)];
            setColorUint(renderer, pixel);
            SDL_RenderDrawPoint(renderer, x+sx, y+sy);
        }
    }
}

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderFillRect(renderer, &rect);
}

uint texture_get_pixel(const uint tex[], uint x, uint y) {
    uint width = tex[1];
    return tex[4+((y*width)+x)];
}

// Set the SDL color from an ARGB uint32
void setColorUint(SDL_Renderer *renderer, uint color) {
    SDL_SetRenderDrawColor(renderer, (color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF, (color >> 24) & 0xFF);
}
