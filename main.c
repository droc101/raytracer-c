#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include "Helpers/Graphics/Drawing.h"
#include "defines.h"
#include "Helpers/Core/Input.h"
#include "Structs/Level.h"
#include "Structs/GlobalState.h"
#include "GameStates/GLogoSplashState.h"
#include "Debug/FrameGrapher.h"
#include "Debug/DPrint.h"
#include "Assets/AssetReader.h"
#include "Helpers/Core/Timing.h"
#include "config.h"
#include "Helpers/Core/Error.h"
#include "Helpers/CommonAssets.h"
#include "Helpers/Graphics/RenderingHelpers.h"
#include "Helpers/Vulkan.h"

int main(int argc, char *argv[]) {

    printf("Build time: %s at %s\n", __DATE__, __TIME__);
    printf("Version: %s\n", VERSION);
    printf("Initializing Engine\n");

    SetSignalHandler(); // catch exceptions in release mode

#ifdef __LINUX__
    setenv("SDL_VIDEODRIVER", "wayland", 1);
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    InitState();
    // TODO: You now have GetState()->options.renderer to determine the renderer. Use this.

    RenderPreInit();

    Mix_AllocateChannels(SFX_CHANNEL_COUNT);

    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) < 0) {
        printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
        return 1;
    }

    Uint32 rendererFlags = GetState()->options.renderer == RENDERER_OPENGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
    SDL_Window *w = SDL_CreateWindow(GAME_TITLE,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,DEF_WIDTH, DEF_HEIGHT, rendererFlags | SDL_WINDOW_RESIZABLE);
    if (w == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowFullscreen(w, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SetWindow(w);
    UpdateViewportSize();

    RenderInit();

    SDL_SetWindowMinimumSize(w, MIN_WIDTH, MIN_HEIGHT);
    SDL_SetWindowMaximumSize(w, MAX_WIDTH, MAX_HEIGHT);

    SDL_Surface *icon = ToSDLSurface((const unsigned char *) gztex_interface_icon, "1");
    SDL_SetWindowIcon(w, icon);

    InitCommonAssets();

    ChangeLevelByID(STARTING_LEVEL);

    GLogoSplashStateSet();

    InitTimers();

    printf("Engine initialized, entering mainloop\n");

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        const ulong frameStart = GetTimeNs();

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
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {

                    UpdateViewportSize();
                }
            }
        }
        ClearDepthOnly();

        ResetDPrintYPos();

        GlobalState *g = GetState();

#ifndef KEYBOARD_ROTATION
        SDL_SetRelativeMouseMode(g->currentState == MAIN_STATE ? SDL_TRUE : SDL_FALSE);
        // warp the mouse to the center of the screen if we are in the main game state
        if (g->currentState == MAIN_STATE) {
            SDL_WarpMouseInWindow(GetWindow(), WindowWidth() / 2, WindowHeight() / 2);
        }
#endif

        if (g->UpdateGame) g->UpdateGame(g);

        g->cam->x = (float)g->level->position.x;
        g->cam->y = (float)g->CameraY;
        g->cam->z = (float)g->level->position.y;
        g->cam->yaw = (float)g->level->rotation;

        g->RenderGame(g);

        FrameGraphDraw();

        Swap();

        UpdateInputStates();

        if (g->requestExit) {
            quit = true;
        }

        FrameGraphUpdate(GetTimeNs() - frameStart);

    }
    printf("Mainloop exited, cleaning up engine...\n");
    DestroyGlobalState();
    SDL_DestroyWindow(GetWindow());
    SDL_FreeSurface(icon);
    InvalidateAssetCache(); // Free all assets
    RenderDestroy();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_Quit();
    return 0;
}

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
#ifdef WIN32
__declspec(dllexport) uint NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif