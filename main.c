#include <SDL.h>
#include <SDL_mixer.h>
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
#include "Assets/AssetReader.h"
#include "Structs/Vector2.h"
#include "Helpers/Timing.h"
#include "config.h"
#include "Helpers/Error.h"

int main(int argc, char *argv[]) {
    printf("Build time: %s at %s\n", __DATE__, __TIME__);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }

    Mix_AllocateChannels(SFX_CHANNEL_COUNT);

    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) < 0) {
        printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow("game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,DEF_WIDTH, DEF_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (w == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetWindow(w);

    SDL_SetWindowMinimumSize(w, 800, 600);
    SDL_SetWindowMaximumSize(w, 8192, 8192);

    SDL_Surface *icon = ToSDLSurface((const unsigned char *) gztex_interface_icon, FILTER_LINEAR);
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

#pragma region Debug Level
    // TODO: Level editor so I don't have to write bytecode by hand
    byte levelData[] = { 0x00, 0xc0, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf9, 0x21, 0xfb, 0x55, 0x20, 0x6d, 0xdf, 0x03, 0xff, 0xeb, 0x40, 0x34, 0xff, 0x33, 0x28, 0x00, 0x04};

    Level *l = LoadLevel(levelData);
//    l->MusicID = 0;

    l->FogColor = 0xFF8040FF;
    l->FogStart = 5;
    l->FogEnd = 20;

    Actor *a = CreateActor(vec2(5, -1), 0, 1);
    ListAdd(l->actors, a);

    Wall *wl = CreateWall(vec2(20, -10), vec2(1, 10), 0);
    ListAdd(l->walls, wl);

    Wall *wl2 = CreateWall(vec2(20, 10), vec2(1, 10), 0);
    ListAdd(l->walls, wl2);

//#define r (rand() % 2 ? rand() % 20 + 1 : -(rand() % 20 + 1))
//
//    for (int i = 3; i < 2700; i++) {
//        Wall *wall = CreateWall(vec2(r, r), vec2(r, r), 0);
//        ListAdd(l->walls, wall);
//    }

    ChangeLevel(l);
#pragma endregion

    GMenuStateSet();

    SDL_Event e;
    bool quit = false;
    ulong frameStart, frameTime;
    while (!quit) {
        frameStart = GetTimeNs();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYUP) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyUp(scancode);
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyDown(scancode);
            } else if (e.type == SDL_MOUSEMOTION) {
                HandleMouseMotion(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                HandleMouseDown(e.button.button);
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                HandleMouseUp(e.button.button);
            }
        }

        ResetDPrintYPos();

        GlobalState *g = GetState();

        SDL_SetRelativeMouseMode(g->UpdateGame == GMainStateUpdate ? SDL_TRUE : SDL_FALSE);
        // warp the mouse to the center of the screen if we are in the main game state
        if (g->UpdateGame == GMainStateUpdate) {
            SDL_WarpMouseInWindow(GetWindow(), WindowWidth() / 2, WindowHeight() / 2);
        }

        g->UpdateGame();

        g->RenderGame();

        FrameGraphDraw();

        SDL_RenderPresent(GetRenderer());

        UpdateInputStates();
        g->frame++;

        if (g->requestExit) {
            quit = true;
        }

        frameTime = GetTimeNs() - frameStart;
        FrameGraphUpdate(frameTime);
        if (frameTime < TARGET_NS) {
            ulong sleepTime = (TARGET_NS - frameTime) / 1000000;
            SDL_Delay(sleepTime);
        }

    }
    printf("Destructing Engine\n");
    DestroyGlobalState();
    SDL_DestroyRenderer(GetRenderer());
    SDL_DestroyWindow(GetWindow());
    SDL_FreeSurface(icon);
    InvalidateAssetCache(); // Free all assets
    SDL_Quit();
    return 0;
}
