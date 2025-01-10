//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_DRAWING_H
#define GAME_DRAWING_H

#include "../../defines.h"
#include "SDL.h"

/**
 * Set the main window
 * @param w The window to use
 */
void SetGameWindow(SDL_Window *w);

/**
 * Get the main window
 * @return the window
 */
SDL_Window *GetGameWindow();

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
void DrawRect(int x, int y, int w, int h);

/**
 * Convert a texture asset to an SDL_Surface
 * @param texture The texture name
 * @param filterMode Texture filtering mode
 * @return The @c SDL_Surface
 */
SDL_Surface *ToSDLSurface(const char *texture, const char *filterMode);

/**
 * Set the color to draw with
 * @param color Color as uint, @c 0xAARRGGBB
 */
void SetColorUint(uint color);

/**
 * Split a color into its components
 * @param color Color as uint, @c 0xAARRGGBB
 * @return Four byte array with the color components
 */
byte *GetColorUint(uint color);

/**
 * Mix two colors together
 * @param color_a Color A
 * @param color_b Color B
 * @return Color A mixed with Color B
 */
uint MixColors(uint color_a, uint color_b);

/**
 * Set the texture parameters (linear, repeat)
 * @param texture The texture name
 * @param linear Whether to use linear filtering (blurring)
 * @param repeat Whether to repeat the texture
 */
void SetTexParams(const char *texture, bool linear, bool repeat);

/**
 * Draw a line from start to end
 * @param start The start of the line
 * @param end The end of the line
 * @param thickness The thickness of the line
 */
void DrawLine(Vector2 start, Vector2 end, float thickness);

/**
 * Draw a 1px outline of a rectangle
 * @param pos The position
 * @param size The size
 * @param thickness The thickness of the outline
 */
void DrawOutlineRect(Vector2 pos, Vector2 size, float thickness);

/**
 * Draw a texture on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 */
void DrawTexture(Vector2 pos, Vector2 size, const char *texture);

/**
 * Draw a texture on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param color The color to draw with
 */
void DrawTextureMod(Vector2 pos, Vector2 size, const char *texture, uint color);

/**
 * Draw a texture region on a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param region_start The start of the region (in pixels)
 * @param region_end The end of the region (in pixels)
 */
void DrawTextureRegion(Vector2 pos, Vector2 size, const char *texture, Vector2 region_start, Vector2 region_end);

/**
 * Draw a texture region on a rectangle with a color
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @param texture The texture name
 * @param region_start The start of the region (in pixels)
 * @param region_end The end of the region (in pixels)
 * @param color The color to draw with
 */
void DrawTextureRegionMod(Vector2 pos,
						  Vector2 size,
						  const char *texture,
						  Vector2 region_start,
						  Vector2 region_end,
						  uint color);

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
 * @param texture The texture name
 * @return The size of the texture
 */
Vector2 GetTextureSize(const char *texture);

/**
 * Draw a nine patch image to the screen
 * @param pos The position to draw at
 * @param size The size of the output
 * @param output_margins_px The 9patch margins of the output
 * @param texture_margins_px The 9patch margins of the texture
 * @param texture The texture name
 * @warning This is nine draw calls.
 */
void DrawNinePatchTexture(Vector2 pos,
						  Vector2 size,
						  int output_margins_px,
						  int texture_margins_px,
						  const char *texture);

#endif //GAME_DRAWING_H
