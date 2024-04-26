//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../defines.h"

#define FILTER_NEAREST "0" // pixelated texture (nearest neighbor)
#define FILTER_LINEAR "1" // smooth texture (linear)

// Set the renderer to draw to
void SetRenderer(SDL_Renderer *r);

// Get the renderer to draw to
SDL_Renderer *GetRenderer();

// Set the window to draw to
void SetWindow(SDL_Window *w);

// Get the window to draw to
SDL_Window *GetWindow();

// Get the window size
int WindowWidth();
int WindowHeight();

// Draw a rectangle at the given position
void draw_rect(int x, int y, int w, int h);

// Convert a texture from assets.h to an SDL_Texture
SDL_Surface* ToSDLSurface(const unsigned char* imageData, char *filterMode);
SDL_Texture* ToSDLTexture(const unsigned char* imageData, char *filterMode);

// Get the size of a texture
SDL_Point SDL_TextureSize(SDL_Texture *texture);

// Draw a column of a texture to the screen (used in raycaster)
void DrawTextureColumn(SDL_Texture* texture, int sx, int dx, int dy, int dh);

// Set the renderer color from an uint
void setColorUint(uint color);

// Split an uint into its color components
byte* getColorUint(uint color);

// Get a screenshot of the current window
SDL_Texture *GetScreenshot();

#endif //GAME_DRAWING_H
