//
// Created by droc101 on 9/30/2024.
//

#include <stdio.h>
#include "glHelper.h"
#include "cglm/cglm.h"
#include "../RenderingHelpers.h"
#include "../../LevelLoader.h"
#include "../../CommonAssets.h"
#include "../../../Assets/AssetReader.h"
#include "../../Core/DataReader.h"
#include "../../../Structs/GlobalState.h"
#include "../../Core/Logging.h"

SDL_GLContext ctx;

GL_Shader *ui_textured;
GL_Shader *ui_colored;
GL_Shader *wall_generic;
GL_Shader *floor_generic;
GL_Shader *shadow;

GL_Buffer *ui_buffer;
GL_Buffer *wall_buffer;

#define MAX_TEXTURES 128

#if MAX_TEXTURES < ASSET_COUNT
#error MAX_TEXTURES must be greater than or equal to ASSET_COUNT
#endif

GLuint GL_Textures[MAX_TEXTURES];
int GL_NextFreeSlot = 0;
int GL_AssetTextureMap[ASSET_COUNT];
char GL_LastError[512];

void GL_Error(const char *error)
{
    LogError("OpenGL Error: %s\n", error);
    strcpy(GL_LastError, error);
}

bool GL_PreInit()
{
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MSAA_SAMPLES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    memset(GL_AssetTextureMap, -1, sizeof(GL_AssetTextureMap));
    memset(GL_Textures, 0, sizeof(GL_Textures));

    return true;
}

bool GL_Init(SDL_Window *wnd)
{
    LogInfo("Initializing OpenGL\n");

    ctx = SDL_GL_CreateContext(wnd);
    if (ctx == NULL)
    {
        LogError("SDL_GL_CreateContext Error: %s\n", SDL_GetError());
        GL_Error("Failed to create OpenGL context");
        return false;
    }

    SDL_GL_SetSwapInterval(GetState()->options.vsync ? 1 : 0);

    GLenum err;
    glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
    err = glewInit();
    if (err != GLEW_OK)
    {
        SDL_GL_DeleteContext(ctx);
        GL_Error("Failed to start OpenGL. Your GPU or drivers may not support OpenGL 4.6.");
        return false;
    }

    char *hud_textured_fsh = (char *) DecompressAsset(gzshd_GL_hud_textured_f);
    char *hud_textured_vsh = (char *) DecompressAsset(gzshd_GL_hud_textured_v);
    ui_textured = GL_ConstructShader(hud_textured_fsh, hud_textured_vsh);

    char *hud_colored_fsh = (char *) DecompressAsset(gzshd_GL_hud_color_f);
    char *hud_colored_vsh = (char *) DecompressAsset(gzshd_GL_hud_color_v);
    ui_colored = GL_ConstructShader(hud_colored_fsh, hud_colored_vsh);

    char *wall_generic_fsh = (char *) DecompressAsset(gzshd_GL_wall_f);
    char *wall_generic_vsh = (char *) DecompressAsset(gzshd_GL_wall_v);
    wall_generic = GL_ConstructShader(wall_generic_fsh, wall_generic_vsh);

    char *floor_generic_fsh = (char *) DecompressAsset(gzshd_GL_floor_f);
    char *floor_generic_vsh = (char *) DecompressAsset(gzshd_GL_floor_v);
    floor_generic = GL_ConstructShader(floor_generic_fsh, floor_generic_vsh);

    char *shadow_fsh = (char *) DecompressAsset(gzshd_GL_shadow_f);
    char *shadow_vsh = (char *) DecompressAsset(gzshd_GL_shadow_v);
    shadow = GL_ConstructShader(shadow_fsh, shadow_vsh);

    ui_buffer = GL_ConstructBuffer();
    wall_buffer = GL_ConstructBuffer();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GL_DebugMessageCallback, NULL);
#endif

    char *vendor = (char *) glGetString(GL_VENDOR);
    char *renderer = (char *) glGetString(GL_RENDERER);
    char *version = (char *) glGetString(GL_VERSION);
    char *shading_language = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

    LogInfo("OpenGL Initialized\n");
    LogInfo("OpenGL Vendor: %s\n", vendor);
    LogInfo("OpenGL Renderer: %s\n", renderer);
    LogInfo("OpenGL Version: %s\n", version);
    LogInfo("GLSL: %s\n", shading_language);

    return true;
}

GL_Shader *GL_ConstructShader(const char *fsh, const char *vsh)
{
    GLint status;
    char err_buf[512];

    GL_Shader *shd = malloc(sizeof(GL_Shader));

    shd->vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shd->vsh, 1, (const GLchar *const *) &vsh, NULL);
    glCompileShader(shd->vsh);
    glGetShaderiv(shd->vsh, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(shd->vsh, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf) - 1] = '\0';
        Error(err_buf);
    }

    shd->fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shd->fsh, 1, (const GLchar *const *) &fsh, NULL);
    glCompileShader(shd->fsh);
    glGetShaderiv(shd->fsh, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetShaderInfoLog(shd->fsh, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf) - 1] = '\0';
        Error(err_buf);
    }

    shd->program = glCreateProgram();
    glAttachShader(shd->program, shd->vsh);
    glAttachShader(shd->program, shd->fsh);
    glBindFragDataLocation(shd->program, 0, "COLOR");
    glLinkProgram(shd->program);

    glGetProgramiv(shd->program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        glGetProgramInfoLog(shd->program, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf) - 1] = '\0';
        Error(err_buf);
    }

    return shd;
}

void GL_DestroyShader(GL_Shader *shd)
{
    glDeleteShader(shd->vsh);
    glDeleteShader(shd->fsh);
    glDeleteProgram(shd->program);
    free(shd);
}

GL_Buffer *GL_ConstructBuffer()
{
    GL_Buffer *buf = malloc(sizeof(GL_Buffer));

    glGenVertexArrays(1, &buf->vao);
    glGenBuffers(1, &buf->vbo);
    glGenBuffers(1, &buf->ebo);

    return buf;
}

void GL_DestroyBuffer(GL_Buffer *buf)
{
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->vbo);
    glDeleteBuffers(1, &buf->ebo);
    free(buf);
}

inline void GL_ClearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL_ClearColor(const uint color)
{
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;
    const float a = ((color >> 24) & 0xFF) / 255.0f;

    glClearColor(r, g, b, a);

    GL_ClearScreen();
}

inline void GL_ClearDepthOnly()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Swap()
{
    SDL_GL_SwapWindow(GetGameWindow());
}

void GL_DestroyGL()
{
    GL_DestroyShader(ui_textured);
    GL_DestroyShader(ui_colored);
    GL_DestroyShader(wall_generic);
    GL_DestroyShader(floor_generic);
    GL_DestroyShader(shadow);
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    GL_DestroyBuffer(ui_buffer);
    GL_DestroyBuffer(wall_buffer);
    SDL_GL_DeleteContext(ctx);
}

inline float GL_X_TO_NDC(const float x)
{
    return (x / WindowWidth()) * 2.0f - 1.0f;
}

inline float GL_Y_TO_NDC(const float y)
{
    return 1.0f - (y / WindowHeight()) * 2.0f;
}

void GL_DrawRect(const Vector2 pos, const Vector2 size, const uint color)
{
    glUseProgram(ui_colored->program);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_colored->program, "col"), r, g, b, a);

    const Vector2 NDC_pos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
    const Vector2 NDC_pos_end = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


    const float vertices[4][2] = {
        {NDC_pos.x, NDC_pos.y},
        {NDC_pos_end.x, NDC_pos.y},
        {NDC_pos_end.x, NDC_pos_end.y},
        {NDC_pos.x, NDC_pos_end.y}
    };

    const uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawRectOutline(const Vector2 pos, const Vector2 size, const uint color, const float thickness)
{
    if (thickness < 1.0f)
    {
        glEnable(GL_LINE_SMOOTH);
    } else
    {
        glDisable(GL_LINE_SMOOTH);
    }

    glLineWidth(thickness);

    glUseProgram(ui_colored->program);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_colored->program, "col"), r, g, b, a);

    const Vector2 NDC_pos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
    const Vector2 NDC_pos_end = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


    const float vertices[4][2] = {
        {NDC_pos.x, NDC_pos.y},
        {NDC_pos_end.x, NDC_pos.y},
        {NDC_pos_end.x, NDC_pos_end.y},
        {NDC_pos.x, NDC_pos_end.y}
    };

    const uint indices[] = {
        0, 1, 2, 3
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, NULL);
}

GLuint GL_LoadTextureFromAsset(const unsigned char *imageData)
{
    if (AssetGetType(imageData) != ASSET_TYPE_TEXTURE)
    {
        Error("Asset is not a texture");
    }

    byte *Decompressed = DecompressAsset(imageData);

    //uint size = ReadUintA(Decompressed, 0);
    const uint width = ReadUintA(Decompressed, 4);
    const uint height = ReadUintA(Decompressed, 8);
    const uint id = ReadUintA(Decompressed, 12);

    if (id >= ASSET_COUNT)
    {
        Error("Texture ID is out of bounds");
    }

    // if the texture is already loaded, don't load it again
    if (GL_AssetTextureMap[id] != -1)
    {
        if (glIsTexture(GL_Textures[GL_AssetTextureMap[id]]))
        {
            return GL_AssetTextureMap[id];
        }
    }

    const byte *pixelData = Decompressed + (sizeof(uint) * 4);

    const int slot = GL_RegisterTexture(pixelData, width, height);

    //printf("Registered asset %d to slot %d\n", id, slot);

    GL_AssetTextureMap[id] = slot;

    return slot;
}

int GL_RegisterTexture(const unsigned char *pixelData, const int width, const int height)
{
    const int slot = GL_NextFreeSlot;

    glGenTextures(1, &GL_Textures[slot]);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, GL_Textures[slot]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, pixelData);

    GL_NextFreeSlot++;

    return slot;
}

void GL_SetTexParams(const unsigned char *imageData, const bool linear, const bool repeat)
{
    GL_LoadTextureFromAsset(imageData); // make sure the texture is loaded

    byte *Decompressed = DecompressAsset(imageData);

    const uint id = ReadUintA(Decompressed, 12);

    const GLuint tex = GL_Textures[GL_AssetTextureMap[id]];

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
}

void
GL_DrawTexture_Internal(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const uint color, const Vector2 region_start,
                        const Vector2 region_end)
{
    glUseProgram(ui_textured->program);

    const GLuint tex = GL_LoadTextureFromAsset(imageData);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_textured->program, "col"), r, g, b, a);

    glUniform4f(glGetUniformLocation(ui_textured->program, "region"), region_start.x, region_start.y, region_end.x,
                region_end.y);

    glUniform1i(glGetUniformLocation(ui_textured->program, "alb"), tex);

    const Vector2 NDC_pos = v2(GL_X_TO_NDC(pos.x), GL_Y_TO_NDC(pos.y));
    const Vector2 NDC_pos_end = v2(GL_X_TO_NDC(pos.x + size.x), GL_Y_TO_NDC(pos.y + size.y));


    const float vertices[4][4] = {
        {NDC_pos.x, NDC_pos.y, 0.0f, 0.0f},
        {NDC_pos_end.x, NDC_pos.y, 1.0f, 0.0f},
        {NDC_pos_end.x, NDC_pos_end.y, 1.0f, 1.0f},
        {NDC_pos.x, NDC_pos_end.y, 0.0f, 1.0f}
    };

    const uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    const GLint tex_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_DrawTexture(const Vector2 pos, const Vector2 size, const unsigned char *imageData)
{
    GL_DrawTexture_Internal(pos, size, imageData, 0xFFFFFFFF, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureMod(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const uint color)
{
    GL_DrawTexture_Internal(pos, size, imageData, color, v2(-1, 0), v2s(0));
}

inline void GL_DrawTextureRegion(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const Vector2 region_start,
                                 const Vector2 region_end)
{
    GL_DrawTexture_Internal(pos, size, imageData, 0xFFFFFFFF, region_start, region_end);
}

inline void GL_DrawTextureRegionMod(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const Vector2 region_start,
                                    const Vector2 region_end, const uint color)
{
    GL_DrawTexture_Internal(pos, size, imageData, color, region_start, region_end);
}

void GL_DrawLine(const Vector2 start, const Vector2 end, const uint color, const float thickness)
{
    if (thickness < 1.0f)
    {
        glEnable(GL_LINE_SMOOTH);
    } else
    {
        glDisable(GL_LINE_SMOOTH);
    }

    glUseProgram(ui_colored->program);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_colored->program, "col"), r, g, b, a);

    const Vector2 NDC_start = v2(GL_X_TO_NDC(start.x), GL_Y_TO_NDC(start.y));
    const Vector2 NDC_end = v2(GL_X_TO_NDC(end.x), GL_Y_TO_NDC(end.y));

    // Calculate the 2 corner vertices of each point for a thick line
    const float vertices[2][2] = {
        {NDC_start.x, NDC_start.y},
        {NDC_end.x, NDC_end.y}
    };

    const uint indices[] = {
        0, 1
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glLineWidth(thickness);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_DrawWall(const Wall *w, const mat4 *mvp, const mat4 *mdl, const Camera *cam, const Level *l)
{
    glUseProgram(wall_generic->program);

    const GLuint tex = GL_LoadTextureFromAsset(w->tex);

    glUniform1i(glGetUniformLocation(wall_generic->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(wall_generic->program, "MODEL_WORLD_MATRIX"), 1, GL_FALSE,
                       mdl[0][0]); // model -> world
    glUniformMatrix4fv(glGetUniformLocation(wall_generic->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE,
                       mvp[0][0]); // world -> screen

    glUniform1f(glGetUniformLocation(wall_generic->program, "camera_yaw"), cam->yaw);
    glUniform1f(glGetUniformLocation(wall_generic->program, "wall_angle"), w->Angle);

    const uint color = l->FogColor;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(wall_generic->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(wall_generic->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(wall_generic->program, "fog_end"), l->FogEnd);

    float vertices[4][5] = {
        // X Y Z U V
        {w->a.x, 0.5f * w->height, w->a.y, 0.0f, 0.0f},
        {w->b.x, 0.5f * w->height, w->b.y, w->Length, 0.0f},
        {w->b.x, -0.5f * w->height, w->b.y, w->Length, 1.0f},
        {w->a.x, -0.5f * w->height, w->a.y, 0.0f, 1.0f}
    };

    const float uvo = w->uvOffset;
    const float uvs = w->uvScale;
    for (int i = 0; i < 4; i++)
    {
        vertices[i][3] = (vertices[i][3] * uvs) + uvo;
    }

    const uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    const GLint tex_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void
GL_DrawFloor(const Vector2 vp1, const Vector2 vp2, const mat4 *mvp, const Level *l, const unsigned char *texture, const float height, const float shade)
{
    glUseProgram(floor_generic->program);

    const GLuint tex = GL_LoadTextureFromAsset(texture);

    glUniform1i(glGetUniformLocation(floor_generic->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(floor_generic->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE,
                       mvp[0][0]); // world -> screen

    const uint color = l->FogColor;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(floor_generic->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(floor_generic->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(floor_generic->program, "fog_end"), l->FogEnd);

    glUniform1f(glGetUniformLocation(floor_generic->program, "height"), height);
    glUniform1f(glGetUniformLocation(floor_generic->program, "shade"), shade);

    const float vertices[4][2] = {
        // X Y Z U V
        {vp1.x, vp1.y},
        {vp2.x, vp1.y},
        {vp2.x, vp2.y},
        {vp1.x, vp2.y}
    };

    const uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(floor_generic->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawShadow(const Vector2 vp1, const Vector2 vp2, const mat4 *mvp, const mat4 *mdl, const Level *l)
{
    glUseProgram(shadow->program);

    const GLuint tex = GL_LoadTextureFromAsset(gztex_vfx_shadow);

    glUniform1i(glGetUniformLocation(shadow->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(shadow->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE,
                       mvp[0][0]); // world -> screen
    glUniformMatrix4fv(glGetUniformLocation(shadow->program, "MODEL_WORLD_MATRIX"), 1, GL_FALSE,
                       mdl[0][0]); // model -> world

    const uint color = l->FogColor;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(shadow->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(shadow->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(shadow->program, "fog_end"), l->FogEnd);

    const float vertices[4][2] = {
        // X Y Z U V
        {vp1.x, vp1.y},
        {vp2.x, vp1.y},
        {vp2.x, vp2.y},
        {vp1.x, vp2.y}
    };

    const uint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(shadow->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

inline void GL_Enable3D()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_Disable3D()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

inline void GL_UpdateViewportSize()
{
    const Vector2 actualWinSize = ActualWindowSize();
    glViewport(0, 0, actualWinSize.x, actualWinSize.y);
}

void GL_DrawColoredArrays(const float *vertices, const uint *indices, const int quad_count, const uint color)
{
    glUseProgram(ui_colored->program);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_textured->program, "col"), r, g, b, a);

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, quad_count * 16 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_count * 6 * sizeof(uint), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, quad_count * 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexturedArrays(const float *vertices, const uint *indices, const int quad_count, const unsigned char *imageData, const uint color)
{
    glUseProgram(ui_textured->program);

    const GLuint tex = GL_LoadTextureFromAsset(imageData);

    const float a = ((color >> 24) & 0xFF) / 255.0f;
    const float r = ((color >> 16) & 0xFF) / 255.0f;
    const float g = ((color >> 8) & 0xFF) / 255.0f;
    const float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_textured->program, "col"), r, g, b, a);

    glUniform4f(glGetUniformLocation(ui_textured->program, "region"), -1, 0, 0, 0);

    glUniform1i(glGetUniformLocation(ui_textured->program, "alb"), tex);

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, quad_count * 16 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_count * 6 * sizeof(uint), indices, GL_STATIC_DRAW);

    const GLint pos_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    const GLint tex_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, quad_count * 6, GL_UNSIGNED_INT, NULL);
}

void GL_RenderLevel(Level *l, Camera *cam)
{
    GL_Enable3D();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    //glLineWidth(2);

    mat4 *WORLD_VIEW_MATRIX = GetMatrix(cam);
    mat4 *IDENTITY = malloc(sizeof(mat4));
    glm_mat4_identity(*IDENTITY);

    const Vector2 floor_start = v2(l->position.x - 100, l->position.y - 100);
    const Vector2 floor_end = v2(l->position.x + 100, l->position.y + 100);

    GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, wallTextures[l->FloorTexture], -0.5, 1.0);
    if (l->CeilingTexture != 0)
    {
        GL_DrawFloor(floor_start, floor_end, WORLD_VIEW_MATRIX, l, wallTextures[l->CeilingTexture - 1], 0.5, 0.8);
    }

    for (int i = 0; i < l->staticWalls->size; i++)
    {
        GL_DrawWall(SizedArrayGet(l->staticWalls, i), WORLD_VIEW_MATRIX, IDENTITY, cam, l);
    }

    for (int i = 0; i < l->staticActors->size; i++)
    {
        Actor *actor = SizedArrayGet(l->staticActors, i);
        WallBake(actor->actorWall);
        mat4 *actor_xfm = ActorTransformMatrix(actor);
        GL_DrawWall(actor->actorWall, WORLD_VIEW_MATRIX, actor_xfm, cam, l);

        if (actor->showShadow)
        {
            // remove the rotation and y position from the actor matrix so the shadow draws correctly
            glm_rotate(*actor_xfm, actor->rotation, (vec3){0, 1, 0});
            glm_translate(*actor_xfm, (vec3){0, -actor->yPosition, 0});

            GL_DrawShadow(v2s(-0.5 * actor->shadowSize), v2s(0.5 * actor->shadowSize), WORLD_VIEW_MATRIX, actor_xfm, l);
        }

        free(actor_xfm);
    }

    free(WORLD_VIEW_MATRIX);
    free(IDENTITY);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    GL_Disable3D();
}
