//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../defines.h"

//#define FILTER_NEAREST "0" // pixelated texture (nearest neighbor)
//#define FILTER_LINEAR "1" // smooth texture (linear)

///**
// * Set the renderer to draw to
// * @param r The renderer to use
// */
//void SetRenderer(SDL_Renderer *r);
//
///**
// * Get the renderer to draw to
// * @return the renderer
// */
//SDL_Renderer *GetRenderer();

/**
 * Set the main window
 * @param w The window to use
 */
void SetWindow(SDL_Window *w);

/**
 * Get the main window
 * @return the window
 */
SDL_Window *GetWindow();

/**
 * Get the width of the window
 * @return width of the window
 */
int WindowWidth();

/**
 * Get the height of the window
 * @return height of the window
 */
int WindowHeight();

/**
 * Draw a solid color rectangle
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @note Call @c setColorUint before calling this function
 */
void draw_rect(int x, int y, int w, int h);

/**
 * Convert a texture asset to an SDL_Surface
 * @param imageData Decompressed image data
 * @param filterMode Texture filtering mode
 * @return The @c SDL_Surface
 */
SDL_Surface* ToSDLSurface(const unsigned char* imageData, char *filterMode);

/**
 * Set the color to draw with
 * @param color Color as uint, @c 0xAARRGGBB
 */
void setColorUint(uint color);

/**
 * Split a color into its components
 * @param color Color as uint, @c 0xAARRGGBB
 * @return Four byte array with the color components
 */
byte* getColorUint(uint color);

/**
 * Take a screenshot of the current window and return it as a texture
 * @return window screenshot as a texture
 */
SDL_Texture *GetScreenshot();

/**
 * Mix two colors together
 * @param color_a Color A
 * @param color_b Color B
 * @return Color A mixed with Color B
 */
uint MixColors(uint color_a, uint color_b);

void SetTexParams(const unsigned char* imageData, bool linear, bool repeat);

void DrawLine(Vector2 start, Vector2 end);

void DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData);

void DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color);

void DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end);

void DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color);

void ClearColor(uint color);

void ClearScreen();

void ClearDepthOnly();

void Swap();

#endif //GAME_DRAWING_H
