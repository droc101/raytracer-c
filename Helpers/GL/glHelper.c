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

Buffer *ui_buffer;
Buffer *wall_buffer;

GLuint textures[ASSET_COUNT];

void GL_DebugMessageCallback(GLenum source, GLenum type, GLuint id,
                             GLenum severity, GLsizei length,
                             const GLchar *msg, const void *data) {
    char *_source;
    char *_type;
    char *_severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }

    printf("%d: %s of %s severity, raised from %s: %s\n",
           id, _type, _severity, _source, msg);
}

void GL_Init() {
    printf("Initializing OpenGL\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    ctx = SDL_GL_CreateContext(GetWindow());

    SDL_GL_SetSwapInterval(1);

    GLenum err;
    glewExperimental = GL_TRUE; // Please expose OpenGL 3.x+ interfaces
    err = glewInit();
    if (err != GLEW_OK) {
        SDL_GL_DeleteContext(ctx);
        Error("Failed to init GLEW");
    }

    char *hud_textured_fsh = (char *) DecompressAsset(gzfsh_shader_hud_textured);
    char *hud_textured_vsh = (char *) DecompressAsset(gzvsh_shader_hud_textured);
    ui_textured = GL_ConstructShader(hud_textured_fsh, hud_textured_vsh);

    char *hud_colored_fsh = (char *) DecompressAsset(gzfsh_shader_hud_color);
    char *hud_colored_vsh = (char *) DecompressAsset(gzvsh_shader_hud_color);
    ui_colored = GL_ConstructShader(hud_colored_fsh, hud_colored_vsh);

    char *wall_generic_fsh = (char *) DecompressAsset(gzfsh_shader_wall);
    char *wall_generic_vsh = (char *) DecompressAsset(gzvsh_shader_wall);
    wall_generic = GL_ConstructShader(wall_generic_fsh, wall_generic_vsh);

    ui_buffer = GL_ConstructBuffer();
    wall_buffer = GL_ConstructBuffer();

    char *vendor = (char *) glGetString(GL_VENDOR);
    char *renderer = (char *) glGetString(GL_RENDERER);
    char *version = (char *) glGetString(GL_VERSION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glClearColor(0.0f, 0.1f, 0.2f, 1.0f);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GL_DebugMessageCallback, NULL);

    printf("OpenGL Initialized\n");
    printf("OpenGL Vendor: %s\n", vendor);
    printf("OpenGL Renderer: %s\n", renderer);
    printf("OpenGL Version: %s\n", version);
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
        Error("ToSDLSurface: Asset is not a texture");
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

    GLint pos_attr_loc = glGetAttribLocation(ui_colored->program, "VERTEX");
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

void GL_DrawWall(Wall *w, mat4 *mvp) {
    glUseProgram(wall_generic->program);

    GLuint tex = GL_LoadTexture(w->tex);

    glUniform1i(glGetUniformLocation(wall_generic->program, "alb"), tex);

    glUniformMatrix4fv(glGetUniformLocation(wall_generic->program, "MODELVIEW_MATRIX"), 1, GL_FALSE, mvp[0][0]);

    float vertices[4][5] = { // X Y Z U V
            {w->a.x, -0.5f, w->a.y, 0.0f, 0.0f},
            {w->b.x, -0.5f, w->b.y, w->Length, 0.0f},
            {w->b.x, 0.5f, w->b.y, w->Length, 1.0f},
            {w->a.x, 0.5f, w->a.y, 0.0f, 1.0f}
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

    GLint pos_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX");
    glVertexAttribPointer(pos_attr_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) 0);
    glEnableVertexAttribArray(pos_attr_loc);

    GLint tex_attr_loc = glGetAttribLocation(wall_generic->program, "VERTEX_UV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(tex_attr_loc);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void GL_Enable3D() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GL_Disable3D() {
    glDisable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
}

