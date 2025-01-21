//
// Created by droc101 on 10/2/24.
//

#ifndef GAME_RENDERINGHELPERS_H
#define GAME_RENDERINGHELPERS_H

#include "../../defines.h"
#include "cglm/cglm.h"
#include "Vulkan/Vulkan.h"

typedef struct BatchedQuadArray BatchedQuadArray;

struct BatchedQuadArray
{
	/// If used in a textured quad, @c verts takes the form of a @c float[quad_count * 16] holding values for X, Y, U, and V for each vertex.
	/// If used in a colored quad, @c verts takes the form of a @c float[quad_count * 8] holding values for X and Y for each vertex.
	float *verts;
	/// uint[quad_count * 6] with indices
	uint *indices;
	/// Number of quads in the array
	int quad_count;
};

extern Renderer currentRenderer;

/**
 * Get the transformation matrix for an actor
 * @param Actor The actor
 * @param transformMatrix A mat4 MODEL matrix of the actor (Model space to world space)
 */
void ActorTransformMatrix(const Actor *Actor, mat4 *transformMatrix);

/**
 * Perform any pre-initialization for the rendering system
 * This is called before the window is created.
 */
bool RenderPreInit();

/**
 * Initialize the rendering system
 */
bool RenderInit();

/**
 * Destroy the rendering system
 */
void RenderDestroy();

/**
 * Run tasks that need to be run before any drawing can be done
 */
VkResult FrameStart();

/**
 * Run tasks needed to present the frame to the screen, as well as swapping the framebuffers
 */
void FrameEnd();

void LoadLevelWalls(const Level *l);

/**
 * Render the 3D portion of a level
 * @param l The level to render
 * @param cam The camera to render with
 * @note - This does not render the sky
 * @note - This destroys the contents of the depth buffer
 */
void RenderLevel3D(const Level *l, const Camera *cam);

/**
 * Update the viewport size
 */
void UpdateViewportSize();

/**
 * Handle minimizing the window
 */
void WindowObscured();

/**
 * Handle restoring the window from minimized state
 */
void WindowRestored();

/**
 * Enable or disable low FPS mode
 * @param val A boolean representing if low FPS mode should be enabled
 */
void SetLowFPS(bool val);

/**
 * Check if low FPS mode is enabled
 * @return A boolean representing if low FPS is enabled
 */
bool IsLowFPSModeEnabled();

/**
 * Gets the supported MSAA levels
 * @return A bitmask of supported MSAA levels
 */
byte GetSampleCountFlags();

/**
 * Draw a `BatchedQuadArray` to the screen using the textured shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param texture The texture name
 * @param color The color to use
 */
void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *texture, uint color);

/**
 * Draw a `BatchedQuadArray` to the screen using the solid color shader. This is faster than multiple draw calls, but harder to use.
 * @param batch The batch to draw
 * @param color The color to use
 */
void DrawBatchedQuadsColored(const BatchedQuadArray *batch, uint color);

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

/**
 * Render the background of the menu screen (main menu, options, level select, etc.)
 */
void RenderMenuBackground();

/**
 * Render the background of the in-game menu (pause, in-game options, etc.)
 */
void RenderInGameMenuBackground();

/**
 * Render a blur-background rectangle
 * @param pos The position of the rectangle in pixels
 * @param size The size of the rectangle in pixels
 * @param blurRadius The radius of the blur in pixels
 * @note This is a very slow operation, use sparingly
 */
void DrawBlur(Vector2 pos, Vector2 size, int blurRadius);

#endif //GAME_RENDERINGHELPERS_H
