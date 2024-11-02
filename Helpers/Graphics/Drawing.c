//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "../../defines.h"
#include "SDL.h"
#include "Drawing.h"
#include "RenderingHelpers.h"
#include "../Core/Error.h"
#include "../../Assets/AssetReader.h"
#include "../LevelLoader.h" // for ReadUInt
#include "GL/glHelper.h"
#include "../../Structs/GlobalState.h"

SDL_Window *window;

uint drawColor = 0xFFFFFFFF;

void SetWindow(SDL_Window *w) {
    window = w;
}

inline SDL_Window *GetWindow() {
    return window;
}

inline int WindowWidth() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    w /= GetState()->options.uiScale;
    return w;
}

inline int WindowHeight() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    h /= GetState()->options.uiScale;
    return h;
}

inline Vector2 ActualWindowSize() {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return v2(w, h);
}

// Set the SDL color from an ARGB uint32
inline void setColorUint(uint color) {
    drawColor = color;
}

byte* getColorUint(uint color) {
    byte *buf = malloc(4);
    buf[0] = (color >> 16) & 0xFF;
    buf[1] = (color >> 8) & 0xFF;
    buf[2] = (color >> 0) & 0xFF;
    buf[3] = (color >> 24) & 0xFF;
    return buf;
}

SDL_Surface* ToSDLSurface(const unsigned char* imageData, char *filterMode) {

    if (AssetGetType(imageData) != ASSET_TYPE_TEXTURE) {
        printf("Asset is not a texture\n");
        Error("ToSDLSurface: Asset is not a texture");
    }

    byte *Decompressed = DecompressAsset(imageData);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filterMode);

    //uint size = ReadUintA(Decompressed, 0);
    uint width = ReadUintA(Decompressed, 4);
    uint height = ReadUintA(Decompressed, 8);
    //uint id = ReadUintA(Decompressed, 12);

    const byte* pixelData = Decompressed + (sizeof(uint) * 4); // Skip the first 4 bytes

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom((void*)pixelData, width, height, 32, width * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!surface) {
        printf("Failed to create surface: %s\n", SDL_GetError());
        Error("ToSDLTexture: Failed to create surface");
    }

    return surface;
}

uint MixColors(uint color_a, uint color_b) {
    // Mix color_a onto color_b, accounting for the alpha of color_a
    byte *a = getColorUint(color_a);
    byte *b = getColorUint(color_b);

    uint r = (a[0] * a[3] + b[0] * (255 - a[3])) / 255;
    uint g = (a[1] * a[3] + b[1] * (255 - a[3])) / 255;
    uint bl = (a[2] * a[3] + b[2] * (255 - a[3])) / 255;
    uint al = a[3] + b[3] * (255 - a[3]) / 255;

    free(a);
    free(b);

    return (r << 16) | (g << 8) | bl | (al << 24);
}

// Rendering subsystem abstractions

void SetTexParams(const unsigned char* imageData, bool linear, bool repeat) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_SetTexParams(imageData, linear, repeat);
            break;
    }
}

inline void DrawLine(Vector2 start, Vector2 end, float thickness) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawLine(start, end, drawColor, thickness * GetState()->options.uiScale);
            break;
    }
}

inline void DrawOutlineRect(Vector2 pos, Vector2 size, float thickness) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawRectOutline(pos, size, drawColor, thickness * GetState()->options.uiScale);
            break;
    }
}

inline void DrawTexture(Vector2 pos, Vector2 size, const unsigned char* imageData) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawTexture(pos, size, imageData);
            break;
    }

}
inline void DrawTextureMod(Vector2 pos, Vector2 size, const unsigned char* imageData, uint color) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawTextureMod(pos, size, imageData, color);
            break;
    }
}

inline void DrawTextureRegion(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawTextureRegion(pos, size, imageData, region_start, region_end);
            break;
    }
}

inline void DrawTextureRegionMod(Vector2 pos, Vector2 size, const unsigned char* imageData, Vector2 region_start, Vector2 region_end, uint color) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawTextureRegionMod(pos, size, imageData, region_start, region_end, color);
            break;
    }
}

inline void ClearColor(uint color) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_ClearColor(color);
            break;
    }
}

inline void ClearScreen() {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_ClearScreen();
            break;
    }

}

inline void ClearDepthOnly() {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_ClearDepthOnly();
            break;
    }
}

inline void Swap() {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_Swap();
            break;
    }
}

inline void draw_rect(int x, int y, int w, int h) {
    switch (currentRenderer) {
        case RENDERER_VULKAN:
            
            break;
        case RENDERER_OPENGL:
            GL_DrawRect(v2(x, y), v2(w, h), drawColor);
            break;
    }
}

Vector2 texture_size(const unsigned char *imageData) {
    byte *Decompressed = DecompressAsset(imageData);

    uint width = ReadUintA(Decompressed, 4);
    uint height = ReadUintA(Decompressed, 8);

    return v2(width, height);
}

