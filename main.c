#include <SDL.h>
#include <stdio.h>
#include "Helpers/drawing.h"
#include "defines.h"
#include "input.h"
#include "Helpers/font.h"
#include "assets/assets.h"
#include "Structs/level.h"
#include "Structs/ray.h"
#include "Helpers/mathex.h"
#include "error.h"
#include "Helpers/LevelLoader.h"
#include "Structs/GlobalState.h"
#include "GameStates/GMenuState.h"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL Initialized\n");

    SDL_Window *window = SDL_CreateWindow("game",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, 0);
    if (window == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Window Created\n");

    SDL_Renderer *tr = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (tr == NULL) {
        SDL_DestroyWindow(window);
        printf("SCreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    printf("Renderer Created\n");
    SetRenderer(tr);
    printf("Renderer Set\n");

    FontInit();
    printf("Font Initialized\n");
    InitState();
    printf("GlobalState Initialized\n");

    byte levelData[] = { 0x00, 0xc0, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf9, 0x21, 0xfb, 0x55, 0x20, 0x6d, 0xdf, 0x03, 0xff, 0xeb, 0x40, 0x34, 0xff, 0x33, 0x28, 0x00, 0x04};

    Level *l = LoadLevel(levelData);
    printf("Level Loaded\n");
    ChangeLevel(l);
    printf("Level Set\n");

    GMenuStateSet();
    printf("GMainState Set\n");

    SDL_Event e;
    bool quit = false;
    ulong frameStart, frameTime;

    ulong frameCount = 0;
    printf("Entering main loop\n");
    while (!quit) {
        frameStart = SDL_GetTicks64();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYUP) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyUp(scancode);
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyDown(scancode);
            }
        }

        GlobalState *g = GetState();

        g->UpdateGame();
        g->RenderGame();

        SDL_RenderPresent(GetRenderer());

        UpdateKeyStates();
        frameCount++;

        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < TARGET_MS) {
            SDL_Delay(TARGET_MS - frameTime);
        }

        // Calculate and print FPS
        //float fps = 1000.0 / (SDL_GetTicks64() - frameStart);
        //sprintf(buffer, "GAME! - FPS: %.2f Time: %lu ms\n", fps, frameTime);
        //SDL_SetWindowTitle(window, buffer);

    }
    printf("Exited main loop\n");


    DestroyLevel(l);
    printf("Level Destroyed\n");

    SDL_DestroyRenderer(GetRenderer());
    printf("Renderer Destroyed\n");
    SDL_DestroyWindow(window);
    printf("Window Destroyed\n");
    SDL_Quit();
    printf("SDL Quit\n");

    return 0;
}