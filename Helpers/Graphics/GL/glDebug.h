//
// Created by droc101 on 10/2/2024.
//

#ifndef NDEBUG

#ifndef GAME_GLDEBUG_H
#define GAME_GLDEBUG_H

#include <GL/glew.h>

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
void GL_DebugMessageCallback(GLenum source, GLenum type, GLuint id,
                             GLenum severity, GLsizei length,
                             const GLchar *msg, const void *data);

#endif //GAME_GLDEBUG_H

#endif //NDEBUG
