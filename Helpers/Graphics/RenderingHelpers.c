//
// Created by droc101 on 10/2/24.
//

#include "RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetReader.h"
#include "../Core/Error.h"
#include "../Core/MathEx.h"
#include "GL/GLHelper.h"

Renderer currentRenderer;
bool lowFPSMode;

void ActorTransformMatrix(const Actor *Actor, mat4 *transformMatrix)
{
	if (!transformMatrix)
	{
		Error("A NULL transformMatrix must not be passed to ActorTransformMatrix!");
	}
	glm_translate(*transformMatrix, (vec3){(float)Actor->position.x, Actor->yPosition, (float)Actor->position.y});
	glm_rotate(*transformMatrix, (float)-Actor->rotation, (vec3){0, 1, 0});
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
			return VK_Init(GetGameWindow());
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
			VK_Cleanup();
			break;
		case RENDERER_OPENGL:
			GL_DestroyGL();
			break;
		default:
			break;
	}
}

VkResult FrameStart()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return VK_FrameStart();
		case RENDERER_OPENGL:
		default:
			return VK_SUCCESS;
	}
}

void FrameEnd()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_FrameEnd();
			break;
		case RENDERER_OPENGL:
			GL_Swap();
			break;
		default:
			break;
	}
}

void LoadLevelWalls(const Level *l)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_LoadLevelWalls(l);
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

void LoadNewActor()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_LoadNewActor();
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

void RenderLevel3D(const Level *l, const Camera *cam)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_RenderLevel(l, cam);
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
	UpdateWindowSize();
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_UpdateViewportSize();
			break;
		default:
			break;
	}
}

inline void WindowObscured()
{
	lowFPSMode = true;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_Minimize();
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

inline void WindowRestored()
{
	lowFPSMode = false;
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_Restore();
			break;
		case RENDERER_OPENGL:
		default:
			break;
	}
}

inline void SetLowFPS(const bool val)
{
	lowFPSMode = val;
}

inline bool IsLowFPSModeEnabled()
{
	return lowFPSMode && GetState()->options.limitFpsWhenUnfocused;
}

inline byte GetSampleCountFlags()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			return VK_GetSampleCountFlags();
		case RENDERER_OPENGL:
			return 0b1111;
		default:
			return 1;
	}
}

inline void DrawBatchedQuadsTextured(const BatchedQuadArray *batch, const char *texture, const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadsBatched(batch->verts, batch->quad_count, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTexturedArrays(batch->verts, batch->indices, batch->quad_count, texture, color);
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
			VK_DrawColoredQuadsBatched(batch->verts, batch->quad_count, color);
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
		case RENDERER_VULKAN: // NOLINT(*-branch-clone)
			return VK_X_TO_NDC(x);
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
			return VK_Y_TO_NDC(y);
		case RENDERER_OPENGL:
			return GL_Y_TO_NDC(y);
		default:
			return 0;
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

	SetColorUint(0xA0000000);
	DrawRect(0, 0, WindowWidth(), WindowHeight());
}
