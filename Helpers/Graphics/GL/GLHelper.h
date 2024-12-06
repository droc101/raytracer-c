//
// Created by droc101 on 9/30/2024.
//

#ifndef GAME_GLHELPER_H
#define GAME_GLHELPER_H

#include <cglm/cglm.h>
#include "SDL.h"
#include "../Drawing.h"

/**
 * Set SDL_GL flags (this must be done before the SDL window is created)
 */
bool GL_PreInit();

/**
 * Initialize OpenGL
 */
bool GL_Init(SDL_Window *wnd);

/**
 * Clear the screen
 */
void GL_ClearScreen();

/**
 * Clear only the depth buffer
 */
void GL_ClearDepthOnly();

/**
 * Swap the buffers
 */
void GL_Swap();

/**
 * Destroy the GL state
 */
void GL_DestroyGL();

/**
 * Set the filter/repeat parameters for a texture
 * @param imageData The texture data
 * @param linear Whether to use linear filtering
 * @param repeat Whether to repeat the texture
 */
void GL_SetTexParams(const unsigned char *imageData, bool linear, bool repeat);

/**
 * Draw a rectangle
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param color The color to draw
 */
void GL_DrawRect(Vector2 pos, Vector2 size, uint color);

/**
 * Draw a rectangle outline
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param color The color to draw
 * @param thickness The thickness of the outline
 */
void GL_DrawRectOutline(Vector2 pos, Vector2 size, uint color, float thickness);

/**
 * Draw a line
 * @param start The start position in pixels
 * @param end The end position in pixels
 * @param color The color to draw
 * @param thickness The thickness of the line
 */
void GL_DrawLine(Vector2 start, Vector2 end, uint color, float thickness);

/**
 * Draw a texture in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 */
void GL_DrawTexture(Vector2 pos, Vector2 size, const unsigned char *imageData);

/**
 * Draw a texture in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param color The modulate color
 */
void GL_DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char *imageData, uint color);

/**
 * Draw a texture region in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param region_start The start of the region in pixels
 * @param region_end The end of the region in pixels
 */
void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char *imageData, Vector2 region_start,
                          Vector2 region_end);

/**
 * Draw a texture region in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param region_start The start of the region in pixels
 * @param region_end The end of the region in pixels
 * @param color The modulate color
 */
void GL_DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char *imageData, Vector2 region_start,
                             Vector2 region_end, uint color);

/**
 * Set the clear color
 * @param color The color to clear the screen with
 * @note This does in fact clear the screen
 */
void GL_ClearColor(uint color);

/**
 * Draw a wall in 3D
 * @param w The wall to draw
 * @param mdl The model -> world matrix
 * @param cam The camera
 * @param l The level
 * @note This expects 3D mode to be enabled
 */
void GL_DrawWall(const Wall *w, const mat4 *mdl, const Camera *cam, const Level *l);

/**
 * Draw the floor in 3D
 * @param vp1 The start of the floor
 * @param vp2 The end of the floor
 * @param mvp The world -> screen matrix
 * @param l The level
 * @param texture The texture to use
 * @param height The height of the floor
 * @param shade The shade of the floor
 */
void
GL_DrawFloor(Vector2 vp1, Vector2 vp2, const mat4 *mvp, const Level *l, const unsigned char *texture,
             float height, float shade);

/**
 * Draw a shadow sprite
 * @param vp1 The first vertex
 * @param vp2 The second vertex
 * @param mvp The model -> screen matrix
 * @param mdl The model -> world matrix
 * @param l The level
 */
void GL_DrawShadow(Vector2 vp1, Vector2 vp2, const mat4 *mvp, const mat4 *mdl, const Level *l);

/**
 * Update the viewport size
 */
void GL_UpdateViewportSize();

/**
 * Draw arrays using the ui_textured shader
 * @param vertices Vertex data [x, y, u, v] with UVs in NDC
 * @param indices Index data
 * @param quad_count The number of quads to draw
 * @param imageData The texture to use
 * @param color The modulate color
 */
void GL_DrawTexturedArrays(const float *vertices, const uint *indices, int quad_count,
                           const unsigned char *imageData, uint color);

/**
 * Draw arrays using the ui_colored shader
 * @param vertices Vertex data [x, y] with positions in NDC
 * @param indices Index data
 * @param quad_count The number of quads to draw
 * @param color The color to draw
 */
void GL_DrawColoredArrays(const float *vertices, const uint *indices, int quad_count, uint color);

/**
 * Convert screen X to NDC
 * @param x X position in pixels
 * @return The NDC position
 */
#define GL_X_TO_NDC(x) ((float) (x) / WindowWidth() * 2.0f - 1.0f)

/**
 * Convert screen Y to NDC
 * @param y Y position in pixels
 * @return The NDC position
 */
#define GL_Y_TO_NDC(y) (1.0f - (float) (y) / WindowHeight() * 2.0f)

/**
 * OpenGL code to render the 3D portion of a level
 * @param l The level to render
 * @param cam The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void GL_RenderLevel(const Level *l, const Camera *cam);

/**
 * Render a 3D model
 * @param m The model to render
 * @param MODEL_WORLD_MATRIX The model -> world matrix
 * @param texture The texture to use
 * @param shader The shader to use
 */
void GL_RenderModel(const Model *m, const mat4 *MODEL_WORLD_MATRIX, const byte *texture, ModelShader shader);

#endif //GAME_GLHELPER_H
