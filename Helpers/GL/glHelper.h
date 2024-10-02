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

void GL_Init();

Shader *GL_ConstructShader(char *fsh, char *vsh);
Buffer *GL_ConstructBuffer();

void GL_ClearScreen();

void GL_ClearDepthOnly();

void GL_Swap();

void GL_DestroyGL();

void GL_SetTexParams(const unsigned char* imageData, bool linear, bool repeat);

void GL_DrawRect(Vector2 pos, Vector2 size, uint color);
void GL_DrawLine(Vector2 start, Vector2 end, uint color);

void GL_DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData);
void GL_DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color);
void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end);
void GL_DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color);

void GL_ClearColor(uint color);

void GL_DrawWall(Wall *w, mat4 *mvp);

void GL_Enable3D();
void GL_Disable3D();

#endif //GAME_GLHELPER_H
