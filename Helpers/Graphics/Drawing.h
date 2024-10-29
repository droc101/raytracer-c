//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "SDL.h"
#include "../../defines.h"

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
 * Get the actual size of the window, ignoring UI scale
 * @return The actual size of the window
 */
Vector2 ActualWindowSize();

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
 * Mix two colors together
 * @param color_a Color A
 * @param color_b Color B
 * @return Color A mixed with Color B
 */
uint MixColors(uint color_a, uint color_b);

/**
 * Set the texture parameters (linear, repeat)
 * @param imageData The texture data
 * @param linear Whether to use linear filtering (blurring)
 * @param repeat Whether to repeat the texture
 */
void SetTexParams(const unsigned char* imageData, bool linear, bool repeat);

/**
 * Draw a line from start to end
 * @param start The start of the line
 * @param end The end of the line
 */
void DrawLine(Vector2 start, Vector2 end);

/**
 * Draw a 1px outline of a rectangle
 * @param pos The position
 * @param size The size
 */
void DrawOutlineRect(Vector2 pos, Vector2 size);

/**
 * Draw a texture on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param imageData The texture data
 */
void DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData);

/**
 * Draw a texture on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param imageData The texture data
 * @param color The color to draw with
 */
void DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color);

/**
 * Draw a texture region on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param imageData The texture data
 * @param region_start The start of the region (in pixels)
 * @param region_end The end of the region (in pixels)
 */
void DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end);

/**
 * Draw a texture region on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param imageData The texture data
 * @param region_start The start of the region (in pixels)
 * @param region_end The end of the region (in pixels)
 * @param color The color to draw with
 */
void DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color);

/**
 * Clear the screen with a color
 * @param color The color to clear with
 */
void ClearColor(uint color);

/**
 * Clear the screen with the last used color
 */
void ClearScreen();

/**
 * Clear the depth buffer
 */
void ClearDepthOnly();

/**
 * Swap the buffers
 */
void Swap();

/**
 * Get the size of a texture
 * @param imageData The texture data
 * @return The size of the texture
 */
Vector2 texture_size(const unsigned char *imageData);

#endif //GAME_DRAWING_H
