//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../defines.h"

void SetRenderer(SDL_Renderer *r);
SDL_Renderer *GetRenderer();

void draw_rect(int x, int y, int w, int h);

SDL_Texture* ToSDLTexture(const unsigned char* imageData);
SDL_Point SDL_TextureSize(SDL_Texture *texture);
void DrawTextureColumn(SDL_Texture* texture, int sx, int dx, int dy, int dh);
void setColorUint(uint color);

#endif //GAME_DRAWING_H
