//
// Created by droc101 on 4/21/2024.
//

#include "Drawing.h"
#include <stdio.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "../Core/AssetReader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "GL/GLHelper.h"
#include "RenderingHelpers.h"
#include "SDL.h"
#include "Vulkan/Vulkan.h"

SDL_Window *window;
int windowWidth;
int windowHeight;

uint drawColor = 0xFFFFFFFF;

void SetGameWindow(SDL_Window *w)
{
	window = w;
}

inline SDL_Window *GetGameWindow()
{
	return window;
}

inline int WindowWidth()
{
	return windowWidth;
}

inline int WindowHeight()
{
	return windowHeight;
}

inline void UpdateWindowSize()
{
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	windowWidth = (int)(windowWidth / GetState()->uiScale);
	windowHeight = (int)(windowHeight / GetState()->uiScale);
}

inline Vector2 ActualWindowSize()
{
	int w;
	int h;
	SDL_GetWindowSize(window, &w, &h);
	return v2(w, h);
}

// Set the SDL color from an ARGB uint32
inline void SetColorUint(const uint color)
{
	drawColor = color;
}

byte *GetColorUint(const uint color)
{
	byte *colorBuf = malloc(4);
	CheckAlloc(colorBuf);
	colorBuf[0] = color >> 16 & 0xFF;
	colorBuf[1] = color >> 8 & 0xFF;
	colorBuf[2] = color >> 0 & 0xFF;
	colorBuf[3] = color >> 24 & 0xFF;
	return colorBuf;
}

SDL_Surface *ToSDLSurface(const char *texture, const char *filterMode)
{
	const Image *img = LoadImage(texture);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);

	SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(img->pixelData,
													img->width,
													img->height,
													32,
													img->width * 4,
													0x00ff0000,
													0x0000ff00,
													0x000000ff,
													0xff000000);
	if (surface == NULL)
	{
		LogError("Failed to create surface: %s\n", SDL_GetError());
		Error("Failed to create surface");
	}

	return surface;
}

uint MixColors(const uint color_a, const uint color_b)
{
	// Mix color_a onto color_b, accounting for the alpha of color_a
	byte *a = GetColorUint(color_a);
	byte *b = GetColorUint(color_b);

	const uint r = (a[0] * a[3] + b[0] * (255 - a[3])) / 255;
	const uint g = (a[1] * a[3] + b[1] * (255 - a[3])) / 255;
	const uint bl = (a[2] * a[3] + b[2] * (255 - a[3])) / 255;
	const uint al = a[3] + b[3] * (255 - a[3]) / 255;

	free(a);
	free(b);

	return r << 16 | g << 8 | bl | al << 24;
}

// Rendering subsystem abstractions

void SetTexParams(const char *texture, const bool linear, const bool repeat)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_SetTexParams(texture, linear, repeat);
			break;
		case RENDERER_OPENGL:
			GL_SetTexParams(texture, linear, repeat);
			break;
		default:
			break;
	}
}

inline void DrawLine(const Vector2 start, const Vector2 end, const float thickness)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawLine((int)start.x,
						(int)start.y,
						(int)end.x,
						(int)end.y,
						thickness * (float)GetState()->uiScale,
						drawColor);
			break;
		case RENDERER_OPENGL:
			GL_DrawLine(start, end, drawColor, thickness * GetState()->uiScale);
			break;
		default:
			break;
	}
}

inline void DrawOutlineRect(const Vector2 pos, const Vector2 size, const float thickness)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawRectOutline((int)pos.x,
							   (int)pos.y,
							   (int)size.x,
							   (int)size.y,
							   thickness * (float)GetState()->uiScale,
							   drawColor);
			break;
		case RENDERER_OPENGL:
			GL_DrawRectOutline(pos, size, drawColor, thickness * GetState()->uiScale);
			break;
		default:
			break;
	}
}

inline void DrawTexture(const Vector2 pos, const Vector2 size, const char *texture)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuad((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, texture);
			break;
		case RENDERER_OPENGL:
			GL_DrawTexture(pos, size, texture);
			break;
		default:
			break;
	}
}

inline void DrawTextureMod(const Vector2 pos, const Vector2 size, const char *texture, const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadMod((int)pos.x, (int)pos.y, (int)size.x, (int)size.y, texture, color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureMod(pos, size, texture, color);
			break;
		default:
			break;
	}
}

inline void DrawTextureRegion(const Vector2 pos,
							  const Vector2 size,
							  const char *texture,
							  const Vector2 region_start,
							  const Vector2 region_end)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegion((int)pos.x,
									  (int)pos.y,
									  (int)size.x,
									  (int)size.y,
									  (int)region_start.x,
									  (int)region_start.y,
									  (int)region_end.x,
									  (int)region_end.y,
									  texture);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegion(pos, size, texture, region_start, region_end);
			break;
		default:
			break;
	}
}

inline void DrawTextureRegionMod(const Vector2 pos,
								 const Vector2 size,
								 const char *texture,
								 const Vector2 region_start,
								 const Vector2 region_end,
								 const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawTexturedQuadRegionMod((int)pos.x,
										 (int)pos.y,
										 (int)size.x,
										 (int)size.y,
										 (int)region_start.x,
										 (int)region_start.y,
										 (int)region_end.x,
										 (int)region_end.y,
										 texture,
										 color);
			break;
		case RENDERER_OPENGL:
			GL_DrawTextureRegionMod(pos, size, texture, region_start, region_end, color);
			break;
		default:
			break;
	}
}

inline void ClearColor(const uint color)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_ClearColor(color);
			break;
		case RENDERER_OPENGL:
			GL_ClearColor(color);
			break;
		default:
			break;
	}
}

inline void ClearScreen()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_ClearScreen();
			break;
		default:
			break;
	}
}

inline void ClearDepthOnly()
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			// Unused
			break;
		case RENDERER_OPENGL:
			GL_ClearDepthOnly();
			break;
		default:
			break;
	}
}

inline void DrawRect(const int x, const int y, const int w, const int h)
{
	switch (currentRenderer)
	{
		case RENDERER_VULKAN:
			VK_DrawColoredQuad(x, y, w, h, drawColor);
			break;
		case RENDERER_OPENGL:
			GL_DrawRect(v2(x, y), v2(w, h), drawColor);
			break;
		default:
			break;
	}
}

Vector2 GetTextureSize(const char *texture)
{
	const Image *img = LoadImage(texture);

	return v2(img->width, img->height);
}

void DrawNinePatchTexture(const Vector2 pos,
						  const Vector2 size,
						  const int output_margins_px,
						  const int texture_margins_px,
						  const char *texture)
{
	const Vector2 textureSize = GetTextureSize(texture);

	DrawTextureRegion(pos, v2s(output_margins_px), texture, v2s(0), v2s(texture_margins_px)); // top left
	DrawTextureRegion(v2(pos.x, pos.y + output_margins_px),
					  v2(output_margins_px, size.y - texture_margins_px * 2),
					  texture,
					  v2(0, texture_margins_px),
					  v2(texture_margins_px, textureSize.y - texture_margins_px * 2)); // middle left
	DrawTextureRegion(v2(pos.x, pos.y + size.y - output_margins_px),
					  v2s(output_margins_px),
					  texture,
					  v2(0, textureSize.y - texture_margins_px),
					  v2s(texture_margins_px)); // bottom left

	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y),
					  v2(size.x - texture_margins_px * 2, output_margins_px),
					  texture,
					  v2(texture_margins_px, 0),
					  v2(textureSize.x - texture_margins_px * 2, texture_margins_px)); // top middle
	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + output_margins_px),
					  v2(size.x - texture_margins_px * 2, size.y - texture_margins_px * 2),
					  texture,
					  v2(texture_margins_px, texture_margins_px),
					  v2(textureSize.x - texture_margins_px * 2,
						 textureSize.y - texture_margins_px * 2)); // middle middle
	DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + (size.y - output_margins_px)),
					  v2(size.x - texture_margins_px * 2, output_margins_px),
					  texture,
					  v2(texture_margins_px, textureSize.y - texture_margins_px),
					  v2(textureSize.x - texture_margins_px * 2, texture_margins_px)); // bottom middle

	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y),
					  v2s(output_margins_px),
					  texture,
					  v2(textureSize.x - texture_margins_px, 0),
					  v2s(texture_margins_px)); // top right
	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + output_margins_px),
					  v2(output_margins_px, size.y - texture_margins_px * 2),
					  texture,
					  v2(textureSize.x - texture_margins_px, texture_margins_px),
					  v2(texture_margins_px, textureSize.y - texture_margins_px * 2)); // middle right
	DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + (size.y - output_margins_px)),
					  v2s(output_margins_px),
					  texture,
					  v2(textureSize.x - texture_margins_px, textureSize.y - texture_margins_px),
					  v2s(texture_margins_px)); // bottom right
}
