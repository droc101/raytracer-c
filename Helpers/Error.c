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

_Noreturn void Error_Internal(char* error, const char* file, int line, const char* function) {
    char buf[256];
    sprintf(buf, "Error: %s\n \n%s:%d (%s)", error, file, line, function);
    printf("%s", buf);

    char dbgInfoBuf[256];
    sprintf(dbgInfoBuf, "Engine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s", VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buf, NULL);
    exit(1);
}

_Noreturn void FriendlyError(char* title, char* description) {
    Error(description);
//    SDL_SetTextureColorMod(menu_bg_tex_red, 0x20, 0x20, 0x20);
//    char dbgInfoBuf[256];
//    sprintf(dbgInfoBuf, "Engine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s", VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);
//    SDL_SetRelativeMouseMode(SDL_FALSE); // release mouse
//    while (1) {
//        SDL_Event e;
//        while (SDL_PollEvent(&e)) {
//            if (e.type == SDL_QUIT) {
//                exit(1);
//            }
//        }
//
//        Vector2 bg_tile_size = v2(320, 240);
//        for (int x = 0; x < WindowWidth(); x += bg_tile_size.x) {
//            for (int y = 0; y < WindowHeight(); y += bg_tile_size.y) {
//                SDL_RenderCopy(GetRenderer(), menu_bg_tex_red, NULL, &(SDL_Rect){x, y, bg_tile_size.x, bg_tile_size.y});
//            }
//        }
//
//        DrawTextAligned(title, 32, 0xFFFF8080, v2s(30), v2(WindowWidth() - 60, WindowHeight() - 60), FONT_HALIGN_CENTER, FONT_VALIGN_TOP, false);
//
//        DrawTextAligned(description, 24, 0xFFFFFFFF, vec2(30, 100), v2(WindowWidth() - 60, WindowHeight() - 160), FONT_HALIGN_CENTER, FONT_VALIGN_TOP, true);
//
//        DrawTextAligned(dbgInfoBuf, 16, 0xFF808080, vec2(30, 100), v2(WindowWidth() - 60, WindowHeight() - 160), FONT_HALIGN_CENTER, FONT_VALIGN_BOTTOM, true);
//
//        SDL_RenderPresent(GetRenderer());
//    }
}

void SignalHandler(int sig) {
    if (sig == SIGSEGV) {
        Error("Segmentation Fault");
    } else if (sig == SIGFPE) {
        Error("Floating Point Exception");
    }
}

void SetSignalHandler() {
#ifdef NDEBUG
    signal(SIGSEGV, SignalHandler);
    signal(SIGFPE, SignalHandler);
#endif
}
