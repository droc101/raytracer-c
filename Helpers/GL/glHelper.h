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

void GL_DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData);
void GL_DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color);
void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end);
void GL_DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color);

void GL_ClearColor(uint color);

void GL_DrawWall(Wall *w, mat4 *mvp, mat4 *mdl);

void GL_Enable3D();
void GL_Disable3D();

#endif //GAME_GLHELPER_H
