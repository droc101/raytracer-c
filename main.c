#include <SDL.h>
#include <stdio.h>
#include "Helpers/Drawing.h"
#include "defines.h"
#include "Helpers/Input.h"
#include "Helpers/Font.h"
#include "Structs/Level.h"
#include "Helpers/LevelLoader.h"
#include "Structs/GlobalState.h"
#include "GameStates/GMenuState.h"
#include "Structs/Actor.h"
#include "Debug/FrameGrapher.h"
#include "GameStates/GMainState.h"
#include "Debug/DPrint.h"

#include "Structs/Vector2.h"

int main(int argc, char *argv[]) {
    printf("Build time: %s at %s\n", __DATE__, __TIME__);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow("game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,DEF_WIDTH, DEF_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (w == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetWindow(w);

    SDL_SetWindowMinimumSize(w, 640, 480);
    SDL_SetWindowMaximumSize(w, 8192, 8192);

    SDL_Surface *icon = ToSDLSurface((const unsigned char *) tex_interface_icon, FILTER_LINEAR);
    SDL_SetWindowIcon(w, icon);

    SDL_Renderer *tr = SDL_CreateRenderer(GetWindow(), -1, SDL_RENDERER_ACCELERATED);
    if (tr == NULL) {
        SDL_DestroyWindow(GetWindow());
        printf("SCreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetRenderer(tr);

    printf("Initializing Engine\n");
    FontInit();
    InitState();
    InitSkyTex();

    byte levelData[] = { 0x00, 0xc0, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf9, 0x21, 0xfb, 0x55, 0x20, 0x6d, 0xdf, 0x03, 0xff, 0xeb, 0x40, 0x34, 0xff, 0x33, 0x28, 0x00, 0x04};

    Level *l = LoadLevel(levelData);

    Actor *a = CreateActor(vec2(5, -1), 0, 1);
    ListAdd(l->actors, a);

    Wall *wl = CreateWall(vec2(0, -5), vec2(1, 10), 0);
    ListAdd(l->walls, wl);

    ChangeLevel(l);

    GMenuStateSet();

    SDL_Event e;
    bool quit = false;
    ulong frameStart, frameTime;
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

        ResetDPrintYPos();

        GlobalState *g = GetState();

        g->UpdateGame();

        g->RenderGame();

        FrameGraphDraw();

        SDL_RenderPresent(GetRenderer());

        UpdateKeyStates();
        g->frame++;

        if (g->requestExit) {
            quit = true;
        }

        frameTime = SDL_GetTicks64() - frameStart;
        FrameGraphUpdate(frameTime);
        if (frameTime < TARGET_MS) {
            SDL_Delay(TARGET_MS - frameTime);
        }

    }
    DestroyLevel(l);

    SDL_DestroyRenderer(GetRenderer());
    SDL_DestroyWindow(GetWindow());
    SDL_FreeSurface(icon);
    SDL_Quit();
    return 0;
}