//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include <cglm/cglm.h>
#include "../defines.h"
#include "../config.h"
#include "Drawing.h"
#include "GL/glHelper.h"

/**
 * Get the transformation matrix for a camera
 * @param cam The camera
 * @return A mat4 MODEL_VIEW_PROJECTION matrix of the camera (World space to screen space)
 */
mat4 *GetMatrix(Camera *cam);

/**
 * Get the transformation matrix for an actor
 * @param Actor The actor
 * @return A mat4 MODEL matrix of the actor (Model space to world space)
 */
mat4 *ActorTransformMatrix(Actor *Actor);

/**
 * Initialize the rendering system
 */
void RenderInit();

/**
 * Destroy the rendering system
 */
void RenderDestroy();

/**
 * Render the 3D portion of a level
 * @param l The level to render
 * @param cam The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void RenderLevel3D(Level *l, Camera *cam);

/**
 * Update the viewport size
 */
void UpdateViewportSize();

#endif //GAME_RENDERINGHELPERS_H
