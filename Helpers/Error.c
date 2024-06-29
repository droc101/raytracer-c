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

const char* basename(const char* path) {
    const char* base = strrchr(path, '/');
    return base ? base + 1 : path;
}

_Noreturn void _Error_Internal(char* error, const char* file, int line, const char* function) {
    char buf[256];
    sprintf(buf, "Error: %s\n \n%s:%d (%s)", error, file, line, function);
    printf("%s", buf);

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
        setColorUint(0x80000000);
        draw_rect(20, 20, WindowWidth() - 40, WindowHeight() - 40);
        DrawTextAligned(buf, 32, 0xFFFF0000, vec2s(30), vec2(WindowWidth() - 60, WindowHeight() - 60), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
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
#ifdef __OPTIMIZE__
    signal(SIGSEGV, _SignalHandler);
    signal(SIGFPE, _SignalHandler);
#endif
}
