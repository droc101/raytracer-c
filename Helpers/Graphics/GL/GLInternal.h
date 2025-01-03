//
// Created by droc101 on 11/11/24.
//
// This file is only to be included by OpenGL rendering code.
// Do not include it from outside the "/Helpers/Graphics/GL" folder.
//

#ifndef GLINTERNAL_H
#define GLINTERNAL_H

#include <cglm/cglm.h>
#include <GL/glew.h>
#include "../../../defines.h"

typedef struct GL_Shader GL_Shader;
typedef struct GL_Buffer GL_Buffer;
typedef struct GL_Framebuffer GL_Framebuffer;

struct GL_Shader
{
	GLuint vsh;
	GLuint fsh;
	GLuint program;
};

struct GL_Buffer
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
};

struct GL_Framebuffer
{
	GLuint frameBufferObjet;
	GLuint colorTexture;
	GLuint depthTexture;
	GLuint renderBufferObject;
};

/**
 * The maximum number of textures that can be loaded into OpenGL
 */
#define MAX_TEXTURES 128
#if MAX_TEXTURES < ASSET_COUNT
#error MAX_TEXTURES must be greater than or equal to ASSET_COUNT
#endif

/**
 * Create a framebuffer
 * @param w The width of the framebuffer
 * @param h The height of the framebuffer
 * @return The new framebuffer
 * @note Caller is required to call @c DestroyFrameBuffer when done
 */
GL_Framebuffer *CreateFramebuffer(int w, int h);

/**
 * Resize a framebuffer
 * @param old The old framebuffer (to be destroyed, can be NULL)
 * @return The new framebuffer
 */
GL_Framebuffer *ResizeFrameBuffer(GL_Framebuffer *old);

/**
 * Destroy a framebuffer
 * @param framebuffer The framebuffer to destroy
 */
void DestroyFrameBuffer(GL_Framebuffer *framebuffer);

/**
 * Log an OpenGL error
 * @param error the error message
 */
void GL_Error(const char *error);

/**
 * Create a shader program from assets
 * @param fsh The fragment shader asset
 * @param vsh The vertex shader asset
 * @return The constructed shader or NULLPTR on error
 */
GL_Shader *GL_ConstructShaderFromAssets(const byte *fsh, const byte *vsh);

/**
 * Create a shader program
 * @param fsh The fragment shader source
 * @param vsh The vertex shader source
 * @return The shader struct or NULLPTR on error
 */
GL_Shader *GL_ConstructShader(const char *fsh, const char *vsh);

/**
 * Create a buffer object
 * @note This should be reused as much as possible
 * @return The buffer struct
 */
GL_Buffer *GL_ConstructBuffer();

/**
 * Debug message callback for OpenGL
 * @param source The source of the message
 * @param type The type of the message
 * @param id The ID of the message
 * @param severity The severity of the message
 * @param length The length of the message
 * @param msg The message
 * @param data Extra data
 */
void GL_DebugMessageCallback(GLenum source,
							 GLenum type,
							 GLuint id,
							 GLenum severity,
							 GLsizei length,
							 const GLchar *msg,
							 const void *data);


/**
 * Load and register a texture from an asset
 * @param imageData The asset data (not decompressed)
 * @return The slot the texture was registered in
 */
GLuint GL_LoadTextureFromAsset(const unsigned char *imageData);


/**
 * Register a texture from pixel data
 * @param pixelData The raw RGBA8 pixel data
 * @param width The width of the texture
 * @param height The height of the texture
 * @return The slot the texture was registered in
 */
int GL_RegisterTexture(const unsigned char *pixelData, int width, int height);

/**
 * Set the level parameters for rendering
 * @param mvp The model -> screen matrix
 * @param l The level
 */
void GL_SetLevelParams(const mat4 *mvp, const Level *l);

/**
 * Enable 3D mode
 */
void GL_Enable3D();

/**
 * Disable 3D mode
 */
void GL_Disable3D();


#endif //GLINTERNAL_H
