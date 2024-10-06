//
// Created by droc101 on 9/30/2024.
//

#include "glHelper.h"
#include <stdio.h>
#include "../../Assets/AssetReader.h"
#include "../../Assets/Assets.h"
#include "../../Structs/Vector2.h"
#include "../LevelLoader.h"
#include <cglm/cglm.h>

SDL_GLContext ctx;

Shader *ui_textured;
Shader *ui_colored;
Shader *wall_generic;
Shader *floor_generic;
Shader *shadow;

Buffer *ui_buffer;
Buffer *wall_buffer;

GLuint textures[ASSET_COUNT];

void GL_Init() {
    printf("Initializing OpenGL\n");

    ctx = SDL_GL_CreateContext(GetWindow());

    SDL_GL_SetSwapInterval(1);

    GLenum err;
    glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
    err = glewInit();
    if (err != GLEW_OK) {
        SDL_GL_DeleteContext(ctx);
        Error("Failed to init GLEW");
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

    char *vendor = (char *) glGetString(GL_VENDOR);
    char *renderer = (char *) glGetString(GL_RENDERER);
    char *version = (char *) glGetString(GL_VERSION);
    char *shading_language = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GL_DebugMessageCallback, NULL);
#endif

    printf("OpenGL Initialized\n");
    printf("OpenGL Vendor: %s\n", vendor);
    printf("OpenGL Renderer: %s\n", renderer);
    printf("OpenGL Version: %s\n", version);
    printf("GLSL: %s\n", shading_language);
}

Shader *GL_ConstructShader(char *fsh, char *vsh) {
    GLint status;
    char err_buf[512];

    Shader *shd = malloc(sizeof(Shader));

    shd->vsh = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shd->vsh, 1, (const GLchar *const *) &vsh, NULL);
    glCompileShader(shd->vsh);
    glGetShaderiv(shd->vsh, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(shd->vsh, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf) - 1] = '\0';
        Error(err_buf);
    }

    shd->fsh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shd->fsh, 1, (const GLchar *const *) &fsh, NULL);
    glCompileShader(shd->fsh);
    glGetShaderiv(shd->fsh, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
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
    if (status != GL_TRUE) {
        glGetProgramInfoLog(shd->program, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf) - 1] = '\0';
        Error(err_buf);
    }

    return shd;
}

void GL_UseShader(Shader *shd) {
    glUseProgram(shd->program);
}

void GL_DestroyShader(Shader *shd) {
    glDeleteShader(shd->vsh);
    glDeleteShader(shd->fsh);
    glDeleteProgram(shd->program);
    free(shd);
}

Buffer *GL_ConstructBuffer() {
    Buffer *buf = malloc(sizeof(Buffer));

    glGenVertexArrays(1, &buf->vao);
    glGenBuffers(1, &buf->vbo);
    glGenBuffers(1, &buf->ebo);

    return buf;
}

void GL_DestroyBuffer(Buffer *buf) {
    glDeleteVertexArrays(1, &buf->vao);
    glDeleteBuffers(1, &buf->vbo);
    glDeleteBuffers(1, &buf->ebo);
    free(buf);
}

void GL_ClearScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL_ClearColor(uint color) {
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;

    glClearColor(r, g, b, a);

    GL_ClearScreen();
}

void GL_ClearDepthOnly() {
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL_Swap() {
    SDL_GL_SwapWindow(GetWindow());
}

void GL_DrawBuffer(Buffer *buf, Shader *shd, int count) {
    GL_UseShader(shd);
    glBindVertexArray(buf->vao);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

void GL_DestroyGL() {
    GL_DestroyShader(ui_textured);
    GL_DestroyShader(ui_colored);
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    GL_DestroyBuffer(ui_buffer);
    SDL_GL_DeleteContext(ctx);
}

float X_TO_NDC(float x) {
    return (x / WindowWidth()) * 2.0f - 1.0f;
}

float Y_TO_NDC(float y) {
    return 1.0f - (y / WindowHeight()) * 2.0f;
}

void GL_DrawRect(Vector2 pos, Vector2 size, uint color) {
    glUseProgram(ui_colored->program);

    float a = ((color >> 24) & 0xFF) / 255.0f;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_colored->program, "col"), r, g, b, a);

    Vector2 NDC_pos = v2(X_TO_NDC(pos.x), Y_TO_NDC(pos.y));
    Vector2 NDC_pos_end = v2(X_TO_NDC(pos.x + size.x), Y_TO_NDC(pos.y + size.y));


    float vertices[4][2] = {
            {NDC_pos.x,     NDC_pos.y},
            {NDC_pos_end.x, NDC_pos.y},
            {NDC_pos_end.x, NDC_pos_end.y},
            {NDC_pos.x,     NDC_pos_end.y}
    };

    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_SetTexParams(const unsigned char *imageData, bool linear, bool repeat) {
    byte *Decompressed = DecompressAsset(imageData);

    uint id = ReadUintA(Decompressed, 12);
    glBindTexture(GL_TEXTURE_2D, textures[id]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
}

GLuint GL_LoadTexture(const unsigned char *imageData) {
    if (AssetGetType(imageData) != ASSET_TYPE_TEXTURE) {
        printf("Asset is not a texture\n");
        Error("Asset is not a texture");
    }

    byte *Decompressed = DecompressAsset(imageData);

    //uint size = ReadUintA(Decompressed, 0);
    uint width = ReadUintA(Decompressed, 4);
    uint height = ReadUintA(Decompressed, 8);
    uint id = ReadUintA(Decompressed, 12);

    if (id >= ASSET_COUNT) {
        Error("Texture ID is out of bounds");
    }

    // if the texture is already loaded, don't load it again
    if (textures[id] != 0) {
        return id;
    }

    const byte *pixelData = Decompressed + (sizeof(uint) * 4);

    glGenTextures(1, &textures[id]);
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, textures[id]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, pixelData);

    return id;
}

void
GL_DrawTexture_Internal(Vector2 pos, Vector2 size, const unsigned char *imageData, uint color, Vector2 region_start,
                        Vector2 region_end) {
    glUseProgram(ui_textured->program);

    GLuint tex = GL_LoadTexture(imageData);

    float a = ((color >> 24) & 0xFF) / 255.0f;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_textured->program, "col"), r, g, b, a);

    glUniform4f(glGetUniformLocation(ui_textured->program, "region"), region_start.x, region_start.y, region_end.x,
                region_end.y);

    glUniform1i(glGetUniformLocation(ui_textured->program, "alb"), tex);

    Vector2 NDC_pos = v2(X_TO_NDC(pos.x), Y_TO_NDC(pos.y));
    Vector2 NDC_pos_end = v2(X_TO_NDC(pos.x + size.x), Y_TO_NDC(pos.y + size.y));


    float vertices[4][4] = {
            {NDC_pos.x,     NDC_pos.y,     0.0f, 0.0f},
            {NDC_pos_end.x, NDC_pos.y,     1.0f, 0.0f},
            {NDC_pos_end.x, NDC_pos_end.y, 1.0f, 1.0f},
            {NDC_pos.x,     NDC_pos_end.y, 0.0f, 1.0f}
    };

    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    GLint tex_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawTexture(Vector2 pos, Vector2 size, const unsigned char *imageData) {
    GL_DrawTexture_Internal(pos, size, imageData, 0xFFFFFFFF, v2(-1, 0), v2s(0));
}

void GL_DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char *imageData, uint color) {
    GL_DrawTexture_Internal(pos, size, imageData, color, v2(-1, 0), v2s(0));
}

void GL_DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char *imageData, Vector2 region_start,
                          Vector2 region_end) {
    GL_DrawTexture_Internal(pos, size, imageData, 0xFFFFFFFF, region_start, region_end);
}

void GL_DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char *imageData, Vector2 region_start,
                             Vector2 region_end, uint color) {
    GL_DrawTexture_Internal(pos, size, imageData, color, region_start, region_end);
}

void GL_DrawLine(Vector2 start, Vector2 end, uint color) {
    glUseProgram(ui_colored->program);

    float a = ((color >> 24) & 0xFF) / 255.0f;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_colored->program, "col"), r, g, b, a);

    Vector2 NDC_start = v2(X_TO_NDC(start.x), Y_TO_NDC(start.y));
    Vector2 NDC_end = v2(X_TO_NDC(end.x), Y_TO_NDC(end.y));

    // Calculate the 2 corner vertices of each point for a thick line
    float vertices[2][2] = {
            {NDC_start.x, NDC_start.y},
            {NDC_end.x,   NDC_end.y}
    };

    unsigned int indices[] = {
            0, 1
    };

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, NULL);
}

void GL_DrawWall(Wall *w, mat4 *mvp, mat4 *mdl, Camera *cam, Level *l) {
    glUseProgram(wall_generic->program);

    GLuint tex = GL_LoadTexture(w->tex);

    glUniform1i(glGetUniformLocation(wall_generic->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(wall_generic->program, "MODEL_WORLD_MATRIX"), 1, GL_FALSE, mdl[0][0]); // model -> world
    glUniformMatrix4fv(glGetUniformLocation(wall_generic->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE, mvp[0][0]); // world -> screen

    glUniform1f(glGetUniformLocation(wall_generic->program, "camera_yaw"), cam->yaw);
    glUniform1f(glGetUniformLocation(wall_generic->program, "wall_angle"), w->Angle);

    uint color = l->FogColor;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(wall_generic->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(wall_generic->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(wall_generic->program, "fog_end"), l->FogEnd);

    float vertices[4][5] = { // X Y Z U V
            {w->a.x, 0.5f * w->height, w->a.y, 0.0f, 0.0f},
            {w->b.x, 0.5f * w->height, w->b.y, w->Length, 0.0f},
            {w->b.x, -0.5f * w->height, w->b.y, w->Length, 1.0f},
            {w->a.x, -0.5f * w->height, w->a.y, 0.0f, 1.0f}
    };

    float uvo = w->uvOffset;
    float uvs = w->uvScale;
    for (int i = 0; i < 4; i++) {
        vertices[i][3] = (vertices[i][3] * uvs) + uvo;
    }

    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    GLint tex_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawFloor(Vector2 vp1, Vector2 vp2, mat4 *mvp, Level *l, const unsigned char *texture, float height, float shade) {
    glUseProgram(floor_generic->program);

    GLuint tex = GL_LoadTexture(texture);

    glUniform1i(glGetUniformLocation(floor_generic->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(floor_generic->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE, mvp[0][0]); // world -> screen

    uint color = l->FogColor;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(floor_generic->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(floor_generic->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(floor_generic->program, "fog_end"), l->FogEnd);

    glUniform1f(glGetUniformLocation(floor_generic->program, "height"), height);
    glUniform1f(glGetUniformLocation(floor_generic->program, "shade"), shade);

    float vertices[4][2] = { // X Y Z U V
            {vp1.x, vp1.y},
            {vp2.x, vp1.y},
            {vp2.x, vp2.y},
            {vp1.x, vp2.y}
    };

    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(floor_generic->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_DrawShadow(Vector2 vp1, Vector2 vp2, mat4 *mvp, mat4 *mdl, Level *l) {
    glUseProgram(shadow->program);

    GLuint tex = GL_LoadTexture(gztex_vfx_shadow);

    glUniform1i(glGetUniformLocation(shadow->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(shadow->program, "WORLD_VIEW_MATRIX"), 1, GL_FALSE, mvp[0][0]); // world -> screen
    glUniformMatrix4fv(glGetUniformLocation(shadow->program, "MODEL_WORLD_MATRIX"), 1, GL_FALSE, mdl[0][0]); // model -> world

    uint color = l->FogColor;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform3f(glGetUniformLocation(shadow->program, "fog_color"), r, g, b);

    glUniform1f(glGetUniformLocation(shadow->program, "fog_start"), l->FogStart);
    glUniform1f(glGetUniformLocation(shadow->program, "fog_end"), l->FogEnd);

    float vertices[4][2] = { // X Y Z U V
            {vp1.x, vp1.y},
            {vp2.x, vp1.y},
            {vp2.x, vp2.y},
            {vp1.x, vp2.y}
    };

    unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glBindVertexArray(wall_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, wall_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(shadow->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_Enable3D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL_Disable3D() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL_UpdateViewportSize() {
    glViewport(0, 0, WindowWidth(), WindowHeight());
}

void GL_DrawTexturedArrays(float *vertices, uint *indices, int quad_count, const unsigned char *imageData, uint color) {
    glUseProgram(ui_textured->program);

    GLuint tex = GL_LoadTexture(imageData);

    float a = ((color >> 24) & 0xFF) / 255.0f;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;

    glUniform4f(glGetUniformLocation(ui_textured->program, "col"), r, g, b, a);

    glUniform4f(glGetUniformLocation(ui_textured->program, "region"), -1, 0, 0, 0);

    glUniform1i(glGetUniformLocation(ui_textured->program, "alb"), tex);

    glBindVertexArray(ui_buffer->vao);

    glBindBuffer(GL_ARRAY_BUFFER, ui_buffer->vbo);
    glBufferData(GL_ARRAY_BUFFER, quad_count * 16 * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_buffer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_count * 6 * sizeof(uint), indices, GL_STATIC_DRAW);

    GLint pos_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    GLint tex_attr_loc = glGetAttribLocation(ui_textured->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, quad_count * 6, GL_UNSIGNED_INT, NULL);
}

void GL_GenQuad(Vector2 pos, Vector2 size, Vector2 uv_start, Vector2 uv_end, float *vertices[][4], uint *indices[6], int quad_number) {
    float NDC_pos_x = X_TO_NDC(pos.x);
    float NDC_pos_y = Y_TO_NDC(pos.y);
    float NDC_pos_end_x = X_TO_NDC(pos.x + size.x);
    float NDC_pos_end_y = Y_TO_NDC(pos.y + size.y);

    float verts[4][4] = {
            {NDC_pos_x, NDC_pos_y, uv_start.x, uv_start.y},
            {NDC_pos_end_x, NDC_pos_y, uv_end.x, uv_start.y},
            {NDC_pos_end_x, NDC_pos_end_y, uv_end.x, uv_end.y},
            {NDC_pos_x, NDC_pos_end_y, uv_start.x, uv_end.y}
    };

    unsigned int inds[6] = {
            0, 1, 2,
            0, 2, 3
    };

    for (int i = 0; i < 6; i++) {
        inds[i] += quad_number * 4;
    }

    float *dest_verts = vertices + (sizeof(float[4][4]) * quad_number);
    uint *dest_inds = indices + (sizeof(uint[6]) * quad_number);

    memcpy(vertices, dest_verts, sizeof(verts));
    memcpy(indices, dest_inds, sizeof(inds));
}
