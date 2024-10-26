//
// Created by droc101 on 9/30/2024.
//

#ifndef GAME_GLHELPER_H
#define GAME_GLHELPER_H

#include <SDL.h>
#include <GL/glew.h>
#include "../Drawing.h"
#include "../Error.h"
#include <cglm/cglm.h>
#include "glDebug.h"

typedef struct {
    GLuint vsh;
    GLuint fsh;
    GLuint program;
} Shader;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
} Buffer;


/**
 * Set SDL_GL flags (this must be done before the SDL window is created)
 */
void GL_PreInit();

/**
 * Initialize OpenGL
 */
void GL_Init();

/**
 * Create a shader program
 * @param fsh The fragment shader source
 * @param vsh The vertex shader source
 * @return The shader struct
 */
Shader *GL_ConstructShader(char *fsh, char *vsh);

/**
 * Create a buffer object
 * @note This should be reused as much as possible
 * @return The buffer struct
 */
Buffer *GL_ConstructBuffer();

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
void GL_SetTexParams(const unsigned char* imageData, bool linear, bool repeat);

/**
 * Draw a rectangle
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param color The color to draw
 */
void GL_DrawRect(Vector2 pos, Vector2 size, uint color);

/**
 * Draw a line
 * @param start The start position in pixels
 * @param end The end position in pixels
 * @param color The color to draw
 */
void GL_DrawLine(Vector2 start, Vector2 end, uint color);

/**
 * Draw a texture in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 */
void GL_DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData);

/**
 * Draw a texture in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param color The modulate color
 */
void GL_DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color);

/**
 * Draw a texture region in 2D
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param region_start The start of the region in pixels
 * @param region_end The end of the region in pixels
 */
void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end);

/**
 * Draw a texture region in 2D with a color mod
 * @param pos The position in pixels
 * @param size The size in pixels
 * @param imageData The texture data
 * @param region_start The start of the region in pixels
 * @param region_end The end of the region in pixels
 * @param color The modulate color
 */
void GL_DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color);

/**
 * Set the clear color
 * @param color The color to clear the screen with
 * @note This does in fact clear the screen
 */
void GL_ClearColor(uint color);

/**
 * Draw a wall in 3D
 * @param w The wall to draw
 * @param mvp The world -> screen matrix
 * @param mdl The model -> world matrix
 * @note This expects 3D mode to be enabled
 */
void GL_DrawWall(Wall *w, mat4 *mvp, mat4 *mdl, Camera *cam, Level *l);

/**
 * Draw the floor in 3D
 * @param vp1 The start of the floor
 * @param vp2 The end of the floor
 * @param mvp The world -> screen matrix
 * @param l The level
 * @param texture The texture to use
 * @param height The height of the floor
 */
void GL_DrawFloor(Vector2 vp1, Vector2 vp2, mat4 *mvp, Level *l, const unsigned char *texture, float height, float shade);

/**
 * Draw a shadow sprite
 * @param vp1 The first vertex
 * @param vp2 The second vertex
 * @param mvp The model -> screen matrix
 * @param mdl The model -> world matrix
 * @param l The level
 */
void GL_DrawShadow(Vector2 vp1, Vector2 vp2, mat4 *mvp, mat4 *mdl, Level *l);

/**
 * Enable 3D mode
 */
void GL_Enable3D();

/**
 * Disable 3D mode
 */
void GL_Disable3D();

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
void GL_DrawTexturedArrays(float *vertices, uint *indices, int quad_count, const unsigned char *imageData, uint color);

/**
 * Draw arrays using the ui_colored shader
 * @param vertices Vertex data [x, y] with positions in NDC
 * @param indices Index data
 * @param quad_count The number of quads to draw
 * @param color The color to draw
 */
void GL_DrawColoredArrays(float *vertices, uint *indices, int quad_count, uint color);

/**
 * Convert screen X to NDC
 * @param x X position in pixels
 * @return The NDC position
 */
float GL_X_TO_NDC(float x);

/**
 * Convert screen Y to NDC
 * @param y Y position in pixels
 * @return The NDC position
 */
float GL_Y_TO_NDC(float y);

#endif //GAME_GLHELPER_H
