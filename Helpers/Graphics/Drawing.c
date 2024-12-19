//
// Created by droc101 on 4/21/2024.
//

#include "Drawing.h"
#include <stdio.h>
#include "RenderingHelpers.h"
#include "SDL.h"
#include "../../defines.h"
#include "../../Assets/AssetReader.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "../Core/DataReader.h"
#include "../Core/Error.h"
#include "../Core/Logging.h"
#include "GL/GLHelper.h"

SDL_Window *window;

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
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    w /= GetState()->uiScale;
    return w;
}

inline int WindowHeight()
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    h /= GetState()->uiScale;
    return h;
}

inline Vector2 ActualWindowSize()
{
    int w, h;
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
    byte *buf = malloc(4);
    chk_malloc(buf);
    buf[0] = color >> 16 & 0xFF;
    buf[1] = color >> 8 & 0xFF;
    buf[2] = color >> 0 & 0xFF;
    buf[3] = color >> 24 & 0xFF;
    return buf;
}

SDL_Surface *ToSDLSurface(const unsigned char *imageData, const char *filterMode)
{
    if (AssetGetType(imageData) != ASSET_TYPE_TEXTURE)
    {
        Error("ToSDLSurface: Asset is not a texture");
    }

    const byte *Decompressed = DecompressAsset(imageData);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);

    //uint size = ReadUintA(Decompressed, 0);
    const uint width = ReadUintA(Decompressed, IMAGE_WIDTH_OFFSET);
    const uint height = ReadUintA(Decompressed, IMAGE_HEIGHT_OFFSET);
    //uint id = ReadUintA(Decompressed, 12);

    const byte *pixelData = Decompressed + sizeof(uint) * 4; // Skip the first 4 bytes

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *) pixelData, width, height, 32, width * 4, 0x00ff0000,
                                                    0x0000ff00, 0x000000ff, 0xff000000);
    if (surface == NULL)
    {
        LogError("Failed to create surface: %s\n", SDL_GetError());
        Error("ToSDLTexture: Failed to create surface");
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

void SetTexParams(const unsigned char *imageData, const bool linear, const bool repeat)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_SetTexParams(imageData, linear, repeat);
            break;
        default: break;
    }
}

inline void DrawLine(const Vector2 start, const Vector2 end, const float thickness)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawLine(start, end, drawColor, thickness * GetState()->uiScale);
            break;
        default: break;
    }
}

inline void DrawOutlineRect(const Vector2 pos, const Vector2 size, const float thickness)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawRectOutline(pos, size, drawColor, thickness * GetState()->uiScale);
            break;
        default: break;
    }
}

inline void DrawTexture(const Vector2 pos, const Vector2 size, const unsigned char *imageData)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawTexture(pos, size, imageData);
            break;
        default: break;
    }
}

inline void DrawTextureMod(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const uint color)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawTextureMod(pos, size, imageData, color);
            break;
        default: break;
    }
}

inline void
DrawTextureRegion(const Vector2 pos, const Vector2 size, const unsigned char *imageData, const Vector2 region_start,
                  const Vector2 region_end)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawTextureRegion(pos, size, imageData, region_start, region_end);
            break;
        default: break;
    }
}

inline void DrawTextureRegionMod(const Vector2 pos, const Vector2 size, const unsigned char *imageData,
                                 const Vector2 region_start,
                                 const Vector2 region_end, const uint color)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawTextureRegionMod(pos, size, imageData, region_start, region_end, color);
            break;
        default: break;
    }
}

inline void ClearColor(const uint color)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_ClearColor(color);
            break;
        default: break;
    }
}

inline void ClearScreen()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_ClearScreen();
            break;
        default: break;
    }
}

inline void ClearDepthOnly()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_ClearDepthOnly();
            break;
        default: break;
    }
}

inline void Swap()
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_Swap();
            break;
        default: break;
    }
}

inline void DrawRect(const int x, const int y, const int w, const int h)
{
    switch (currentRenderer)
    {
        case RENDERER_VULKAN:

            break;
        case RENDERER_OPENGL:
            GL_DrawRect(v2(x, y), v2(w, h), drawColor);
            break;
        default: break;
    }
}

Vector2 GetTextureSize(const unsigned char *imageData)
{
    const byte *Decompressed = DecompressAsset(imageData);

    const uint width = ReadUintA(Decompressed, IMAGE_WIDTH_OFFSET);
    const uint height = ReadUintA(Decompressed, IMAGE_HEIGHT_OFFSET);

    return v2(width, height);
}

void DrawNinePatchTexture(const Vector2 pos, const Vector2 size, const int output_margins_px, const int texture_margins_px,
                    const byte *imageData)
{
    const Vector2 ts = GetTextureSize(imageData);
    DrawTextureRegion(pos, v2s(output_margins_px), imageData, v2s(0), v2s(texture_margins_px)); // top left
    DrawTextureRegion(v2(pos.x, pos.y + output_margins_px), v2(output_margins_px, size.y - texture_margins_px * 2),
                      imageData, v2(0, texture_margins_px),
                      v2(texture_margins_px, ts.y - texture_margins_px * 2)); // middle left
    DrawTextureRegion(v2(pos.x, pos.y + size.y - output_margins_px), v2s(output_margins_px), imageData,
                      v2(0, ts.y - texture_margins_px), v2s(texture_margins_px)); // bottom left

    DrawTextureRegion(v2(pos.x + output_margins_px, pos.y), v2(size.x - texture_margins_px * 2, output_margins_px),
                      imageData, v2(texture_margins_px, 0),
                      v2(ts.x - texture_margins_px * 2, texture_margins_px)); // top middle
    DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + output_margins_px),
                      v2(size.x - texture_margins_px * 2, size.y - texture_margins_px * 2), imageData,
                      v2(texture_margins_px, texture_margins_px),
                      v2(ts.x - texture_margins_px * 2, ts.y - texture_margins_px * 2)); // middle middle
    DrawTextureRegion(v2(pos.x + output_margins_px, pos.y + (size.y - output_margins_px)),
                      v2(size.x - texture_margins_px * 2, output_margins_px), imageData,
                      v2(texture_margins_px, ts.y - texture_margins_px),
                      v2(ts.x - texture_margins_px * 2, texture_margins_px)); // bottom middle

    DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y), v2s(output_margins_px), imageData,
                      v2(ts.x - texture_margins_px, 0), v2s(texture_margins_px)); // top right
    DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + output_margins_px),
                      v2(output_margins_px, size.y - texture_margins_px * 2), imageData,
                      v2(ts.x - texture_margins_px, texture_margins_px),
                      v2(texture_margins_px, ts.y - texture_margins_px * 2)); // middle right
    DrawTextureRegion(v2(pos.x + (size.x - output_margins_px), pos.y + (size.y - output_margins_px)),
                      v2s(output_margins_px), imageData, v2(ts.x - texture_margins_px, ts.y - texture_margins_px),
                      v2s(texture_margins_px)); // bottom right
}
