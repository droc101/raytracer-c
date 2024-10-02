//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include <cglm/cglm.h>
#include "../defines.h"
#include "../config.h"
#include "Drawing.h"

mat4 *GL_GetMatrix(Camera *cam);

void RenderInit();

void RenderDestroy();

void RenderLevel3D(Level *l, Camera *cam);

#endif //GAME_RENDERINGHELPERS_H
