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
#include "Helpers/Vulkan.h"

int main(int argc, char *argv[]) {

    printf("Build time: %s at %s\n", __DATE__, __TIME__);
    printf("Version: %s\n", VERSION);
    printf("SDL Version: %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }

    Mix_AllocateChannels(SFX_CHANNEL_COUNT);

    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) < 0) {
        printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow(GAME_TITLE,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,DEF_WIDTH, DEF_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
    if (w == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetWindow(w);

    SDL_SetWindowMinimumSize(w, MIN_WIDTH, MIN_HEIGHT);
    SDL_SetWindowMaximumSize(w, MAX_WIDTH, MAX_HEIGHT);



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
    SetSignalHandler(); // catch exceptions in release mode

    printf("Initializing Engine\n");
    FontInit();
    InitState();
    InitSkyTex();

    ChangeLevelByID(STARTING_LEVEL);

    GMenuStateSet();

    SDL_Event e;
    bool quit = false;
    ulong frameStart, frameTime;

    InitVulkan(w);

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

#ifndef KEYBOARD_ROTATION
        SDL_SetRelativeMouseMode(g->UpdateGame == GMainStateUpdate ? SDL_TRUE : SDL_FALSE);
        // warp the mouse to the center of the screen if we are in the main game state
        if (g->UpdateGame == GMainStateUpdate) {
            SDL_WarpMouseInWindow(GetWindow(), WindowWidth() / 2, WindowHeight() / 2);
        }
#endif

        g->UpdateGame();

        g->RenderGame(GetVulkanInstance(), GetVulkanSurface());

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
    CleanupVulkan();
    DestroyGlobalState();
    SDL_DestroyRenderer(GetRenderer());
    SDL_DestroyWindow(GetWindow());
    SDL_FreeSurface(icon);
    InvalidateAssetCache(); // Free all assets
    SDL_Quit();
    return 0;
}
