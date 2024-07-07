//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "../defines.h"
#include "Error.h"
#include "Drawing.h"
#include "Font.h"
#include <string.h>
#include <signal.h>
#include <zlib.h>
#include "CommonAssets.h"

_Noreturn void _Error_Internal(char* error, const char* file, int line, const char* function) {
    char buf[256];
    sprintf(buf, "Error: %s\n \n%s:%d (%s)", error, file, line, function);
    printf("%s", buf);

    char dbgInfoBuf[256];
    sprintf(dbgInfoBuf, "Engine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s", VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);

    SDL_Texture *bgTexture;
    bgTexture = GetScreenshot();
    SDL_SetRelativeMouseMode(SDL_FALSE); // release mouse
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(1);
            }
        }
        SDL_RenderCopy(GetRenderer(), bgTexture, NULL, NULL);
        SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
        setColorUint(0xa0000000);
        draw_rect(20, 20, WindowWidth() - 40, WindowHeight() - 40);
        DrawTextAligned(buf, 24, 0xFFFF0000, vec2s(30), vec2(WindowWidth() - 60, WindowHeight() - 60), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        DrawTextAligned(dbgInfoBuf, 16, 0xFF808080, vec2(30, 100), vec2(WindowWidth() - 60, WindowHeight() - 160), FONT_HALIGN_CENTER, FONT_VALIGN_BOTTOM, true);
        SDL_RenderPresent(GetRenderer());
    }
}

_Noreturn void FriendlyError(char* title, char* description) {
    SDL_SetTextureColorMod(menu_bg_tex_red, 0x20, 0x20, 0x20);
    char dbgInfoBuf[256];
    sprintf(dbgInfoBuf, "Engine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s", VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);
    SDL_SetRelativeMouseMode(SDL_FALSE); // release mouse
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(1);
            }
        }

        Vector2 bg_tile_size = vec2(320, 240);
        for (int x = 0; x < WindowWidth(); x += bg_tile_size.x) {
            for (int y = 0; y < WindowHeight(); y += bg_tile_size.y) {
                SDL_RenderCopy(GetRenderer(), menu_bg_tex_red, NULL, &(SDL_Rect){x, y, bg_tile_size.x, bg_tile_size.y});
            }
        }

        DrawTextAligned(title, 32, 0xFFFF8080, vec2s(30), vec2(WindowWidth() - 60, WindowHeight() - 60), FONT_HALIGN_CENTER, FONT_VALIGN_TOP, false);

        DrawTextAligned(description, 24, 0xFFFFFFFF, vec2(30, 100), vec2(WindowWidth() - 60, WindowHeight() - 160), FONT_HALIGN_CENTER, FONT_VALIGN_TOP, true);

        DrawTextAligned(dbgInfoBuf, 16, 0xFF808080, vec2(30, 100), vec2(WindowWidth() - 60, WindowHeight() - 160), FONT_HALIGN_CENTER, FONT_VALIGN_BOTTOM, true);

        SDL_RenderPresent(GetRenderer());
    }
}

void _SignalHandler(int sig) {
    if (sig == SIGSEGV) {
        Error("Segmentation Fault");
    } else if (sig == SIGFPE) {
        Error("Floating Point Exception");
    }
}

void SetSignalHandler() {
#ifdef NDEBUG
    signal(SIGSEGV, _SignalHandler);
    signal(SIGFPE, _SignalHandler);
#endif
}
