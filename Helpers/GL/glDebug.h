//
// Created by droc101 on 10/2/2024.
//

#ifndef GAME_GLDEBUG_H
#define GAME_GLDEBUG_H

#include <GL/glew.h>

void GL_DebugMessageCallback(GLenum source, GLenum type, GLuint id,
                             GLenum severity, GLsizei length,
                             const GLchar *msg, const void *data);

#endif //GAME_GLDEBUG_H
