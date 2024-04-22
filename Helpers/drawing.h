//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../defines.h"

// Set the renderer to draw to
void SetRenderer(SDL_Renderer *r);

// Get the renderer to draw to
SDL_Renderer *GetRenderer();

// Draw a rectangle at the given position
void draw_rect(int x, int y, int w, int h);

// Convert a texture from assets.h to an SDL_Texture
SDL_Texture* ToSDLTexture(const unsigned char* imageData, char *filterMode);

// Get the size of a texture
SDL_Point SDL_TextureSize(SDL_Texture *texture);

// Draw a column of a texture to the screen (used in raycaster)
void DrawTextureColumn(SDL_Texture* texture, int sx, int dx, int dy, int dh);

// Set the renderer color from a uint
void setColorUint(uint color);

#endif //GAME_DRAWING_H
