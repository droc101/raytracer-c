//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../Core/Error.h"
#include "../Core/MathEx.h"
#include "GL/GLHelper.h"
#include "../../Structs/Level.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetReader.h"

Renderer currentRenderer;

mat4 *GetMatrix(const Camera *cam)
{
	vec3 cameraPosition = {cam->x, cam->y, cam->z};
	const float aspectRatio = (float)WindowWidth() / (float)WindowHeight();

	mat4 IDENTITY = GLM_MAT4_IDENTITY_INIT;
	mat4 PERSPECTIVE = GLM_MAT4_ZERO_INIT;
	glm_perspective(glm_rad(cam->fov), aspectRatio, NEAR_Z, FAR_Z, PERSPECTIVE);

	vec3 lookAt = {cosf(cam->yaw), 0, sinf(cam->yaw)};
	vec3 up = {0, 1, 0};

	// TODO: roll and pitch are messed up

	glm_vec3_rotate(lookAt, cam->roll, (vec3){0, 0, 1}); // Roll
	glm_vec3_rotate(lookAt, cam->pitch, (vec3){1, 0, 0}); // Pitch

	lookAt[0] += cameraPosition[0];
	lookAt[1] += cameraPosition[1];
	lookAt[2] += cameraPosition[2];

	mat4 VIEW = GLM_MAT4_ZERO_INIT;
	glm_lookat(cameraPosition, lookAt, up, VIEW);

	mat4 MODEL_VIEW = GLM_MAT4_ZERO_INIT;
	glm_mat4_mul(VIEW, IDENTITY, MODEL_VIEW);

	mat4 *MODEL_VIEW_PROJECTION = malloc(sizeof(mat4));
	chk_malloc(MODEL_VIEW_PROJECTION);
	glm_mat4_mul(PERSPECTIVE, MODEL_VIEW, *MODEL_VIEW_PROJECTION);

	return MODEL_VIEW_PROJECTION;
}

mat4 *ActorTransformMatrix(const Actor *Actor)
{
	mat4 *MODEL = malloc(sizeof(mat4));
	chk_malloc(MODEL);
	glm_mat4_identity(*MODEL);
	glm_translate(*MODEL, (vec3){Actor->position.x, Actor->yPosition, Actor->position.y});
	glm_rotate(*MODEL, -Actor->rotation, (vec3){0, 1, 0});
	return MODEL;
}

bool RenderPreInit()
{
	currentRenderer = GetState()->options.renderer;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return true;
		case RENDERER_OPENGL:
			return GL_PreInit();
		default:
			return false;
	}
}

bool RenderInit()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return false;
		case RENDERER_OPENGL:
			return GL_Init(GetGameWindow());
		default:
			return false;
	}
}

void RenderDestroy()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_DestroyGL();
			break;
		default:
			break;
	}
}

void RenderLevel3D(const Level *l, const Camera *cam)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_RenderLevel(l, cam);
			break;
		default:
			break;
	}
}

inline void UpdateViewportSize()
{
	const float newScaleX = (float)ActualWindowSize().x / (float)DEF_WIDTH;
	const float newScaleY = (float)ActualWindowSize().y / (float)DEF_HEIGHT;
	float newScale = newScaleX < newScaleY ? newScaleX : newScaleY;
	newScale = max(newScale, 1.0f);
	GetState()->uiScale = newScale;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_UpdateViewportSize();
			break;
		default:
			break;
	}
}

inline void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *imageData, const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, imageData, color);
			break;
		default:
			break;
	}
}

inline void DrawBatchedQuadsColored(const BatchedQuadArray *batch, const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_DrawColoredArrays(batch->verts, batch->indices, batch->quad_count, color);
			break;
		default:
			break;
	}
}

inline float X_TO_NDC(const float x)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return 0;
		case RENDERER_OPENGL:
			return GL_X_TO_NDC(x);
		default:
			return 0;
	}
}

inline float Y_TO_NDC(const float y)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return 0;
		case RENDERER_OPENGL:
			return GL_Y_TO_NDC(y);
		default:
			return 0;
	}
}

void RenderModel(const Model *m, const mat4 *MODEL_WORLD_MATRIX, const char *texture, const ModelShader shd)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_RenderModel(m, MODEL_WORLD_MATRIX, texture, shd);
		default:
			break;
	}
}

void RenderMenuBackground()
{
	// sorry for the confusing variable names
	const Vector2 bgTileSize = v2(320, 240); // size on screen
	const Vector2 bgTexSize = GetTextureSize(TEXTURE("interface_menu_bg_tile")); // actual size of the texture

	const Vector2 tilesOnScreen = v2(WindowWidth() / bgTileSize.x, WindowHeight() / bgTileSize.y);
	const Vector2 tileRegion = v2(tilesOnScreen.x * bgTexSize.x, tilesOnScreen.y * bgTexSize.y);
	DrawTextureRegion(v2(0, 0),
					  v2(WindowWidth(), WindowHeight()),
					  TEXTURE("interface_menu_bg_tile"),
					  v2(0, 0),
					  tileRegion);
}

void RenderInGameMenuBackground()
{
	RenderLevel(GetState());

	DrawBlur(v2(0, 0), v2(WindowWidth(), WindowHeight()), 4.0f);

	SetColorUint(0x80000000);
	DrawRect(0, 0, WindowWidth(), WindowHeight());
}

void DrawBlur(const Vector2 pos, const Vector2 size, const int blurRadius)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:

			break;
		case RENDERER_OPENGL:
			GL_DrawBlur(pos, size, blurRadius);
		default:
			break;
	}
}
