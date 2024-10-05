
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include "Helpers/Drawing.h"
#include "defines.h"
#include "Helpers/Input.h"
#include "Structs/Level.h"
#include "Structs/GlobalState.h"
#include "GameStates/GLogoSplashState.h"
#include "Debug/FrameGrapher.h"
#include "GameStates/GMainState.h"
#include "Debug/DPrint.h"
#include "Assets/AssetReader.h"
#include "Helpers/Timing.h"
#include "config.h"
#include "Helpers/Error.h"
#include "Helpers/CommonAssets.h"
#include "Helpers/RenderingHelpers.h"
#include "Helpers/MathEx.h"

int main(int argc, char *argv[]) {

    printf("Build time: %s at %s\n", __DATE__, __TIME__);
    printf("Version: %s\n", VERSION);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // OpenGL Window Properties (Don't do this if using Vulkan)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MSAA_SAMPLES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    Mix_AllocateChannels(SFX_CHANNEL_COUNT);

    if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) < 0) {
        printf("Mix_OpenAudio Error: %s\n", Mix_GetError());
        return 1;
    }

    SDL_Window *w = SDL_CreateWindow(GAME_TITLE,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,DEF_WIDTH, DEF_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (w == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetWindow(w);

    RenderInit();

    SDL_SetWindowMinimumSize(w, MIN_WIDTH, MIN_HEIGHT);
    SDL_SetWindowMaximumSize(w, MAX_WIDTH, MAX_HEIGHT);

    SDL_Surface *icon = ToSDLSurface((const unsigned char *) gztex_interface_icon, "1");
    SDL_SetWindowIcon(w, icon);

    SetSignalHandler(); // catch exceptions in release mode

    printf("Initializing Engine\n");
    InitCommonAssets();
    InitState();

    ChangeLevelByID(STARTING_LEVEL);

    GLogoSplashStateSet();

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
        SDL_SetRelativeMouseMode(g->UpdateGame == GMainStateUpdate ? SDL_TRUE : SDL_FALSE);
        // warp the mouse to the center of the screen if we are in the main game state
        if (g->UpdateGame == GMainStateUpdate) {
            SDL_WarpMouseInWindow(GetWindow(), WindowWidth() / 2, WindowHeight() / 2);
        }
#endif

        g->UpdateGame(g);

        g->cam->x = g->level->position.x;
        g->cam->y = g->FakeHeight;
        g->cam->z = g->level->position.y;
        g->cam->yaw = g->level->rotation;

        RenderLevelSky(g->cam);

        RenderLevel3D(g->level, g->cam);

        g->RenderGame(g);

        FrameGraphDraw();

        Swap();

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
    SDL_DestroyWindow(GetWindow());
    SDL_FreeSurface(icon);
    InvalidateAssetCache(); // Free all assets
    RenderDestroy();
    SDL_Quit();
    return 0;
}
