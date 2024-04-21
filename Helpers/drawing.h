//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../defines.h"

void draw_texture(SDL_Renderer *renderer, const uint tex[], uint sx, uint sy);
void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h);

uint texture_get_pixel(const uint tex[], uint x, uint y);
void setColorUint(SDL_Renderer *renderer, uint color);

#endif //GAME_DRAWING_H
