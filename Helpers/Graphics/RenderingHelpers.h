//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include "cglm/cglm.h"
#include "../../defines.h"
#include "../../config.h"
#include "Drawing.h"
#include "GL/glHelper.h"

typedef struct BatchedQuadArray {
    float *verts; // float[quad_count][4] with X,Y,U,V for textured quads, float[quad_count][2] with X,Y for untextured quads
    uint *indices; // uint[6*quad_count] with indices
    int quad_count; // Number of quads in the array
} BatchedQuadArray;

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
 * Perform any pre-initialization for the rendering system
 * This is called before the window is created.
 */
void RenderPreInit();

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

/**
 * Draw a `BatchedQuadArray` to the screen using the textured shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param imageData The texture to use
 * @param color The color to use
 */
void DrawBatchedQuadsTextured(BatchedQuadArray *batch, const unsigned char *imageData, uint color);

/**
 * Draw a `BatchedQuadArray` to the screen using the solid color shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param color The color to use
 */
void DrawBatchedQuadsColored(BatchedQuadArray *batch, uint color);

/**
 * Convert screen X to NDC
 * @param x X position in pixels
 * @return The NDC position
 */
float X_TO_NDC(float x);

/**
 * Convert screen Y to NDC
 * @param y Y position in pixels
 * @return The NDC position
 */
float Y_TO_NDC(float y);

#endif //GAME_RENDERINGHELPERS_H
