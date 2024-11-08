#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "defines.h"
#include "Assets/AssetReader.h"
#include "Debug/DPrint.h"
#include "Debug/FrameGrapher.h"
#include "GameStates/GLogoSplashState.h"
#include "Helpers/CommonAssets.h"
#include "Helpers/Core/Error.h"
#include "Helpers/Core/Input.h"
#include "Helpers/Core/Logging.h"
#include "Helpers/Core/Timing.h"
#include "Helpers/Graphics/Drawing.h"
#include "Helpers/Graphics/RenderingHelpers.h"
#include "Structs/GlobalState.h"
#include "Structs/Level.h"

#include "GameStates/GPauseState.h"

int main(int argc, char *argv[])
{
    LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
    LogInfo("Version: %s\n", VERSION);
    LogInfo("Initializing Engine\n");

    ErrorHandlerInit();

    if (argc < 1) { // this should *never* happen, but let's be safe
        Error("No executable path argument provided.");
    }

    const int argvZeroLen = strlen(argv[0]);

    if (argvZeroLen > 260)
    {
        Error("Executable path too long. Please rethink your file structure.");
    }
    memset(GetState()->executablePath, 0, 261); // we do not mess around with user data in c.
    strncpy(GetState()->executablePath, argv[0], 260);
    LogInfo("Executable path: %s\n", GetState()->executablePath);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        LogError("SDL_Init Error: %s\n", SDL_GetError());
        Error("Failed to initialize SDL");
    }

    InitState();

    if (!RenderPreInit())
    {
        RenderInitError();
    }

    Mix_AllocateChannels(SFX_CHANNEL_COUNT);

    if (Mix_OpenAudio(48000, AUDIO_S16, 2, 2048) < 0)
    {
        LogError("Mix_OpenAudio Error: %s\n", Mix_GetError());
        Error("Failed to initialize audio system.");
    }

    const Uint32 rendererFlags = currentRenderer == RENDERER_OPENGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
    SDL_Window *w = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEF_WIDTH,
                                     DEF_HEIGHT, rendererFlags | SDL_WINDOW_RESIZABLE);
    if (w == NULL)
    {
        LogError("SDL_CreateWindow Error: %s\n", SDL_GetError());
        Error("Failed to create window.");
    }
    DwmDarkMode(w);
    SDL_SetWindowFullscreen(w, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SetWindow(w);
    UpdateViewportSize();

    if (!RenderInit())
    {
        RenderInitError();
    }

    SDL_SetWindowMinimumSize(w, MIN_WIDTH, MIN_HEIGHT);
    SDL_SetWindowMaximumSize(w, MAX_WIDTH, MAX_HEIGHT);

    SDL_Surface *icon = ToSDLSurface(gztex_interface_icon, "1");
    SDL_SetWindowIcon(w, icon);

    InitCommonAssets();

    ChangeLevelByID(STARTING_LEVEL);

    // GLogoSplashStateSet();
    ChangeLevelByID(1);
    GPauseStateSet();

    InitTimers();

    LogInfo("Engine initialized, entering mainloop\n");

    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        const ulong frameStart = GetTimeNs();

        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYUP:
                    HandleKeyUp(e.key.keysym.scancode);
                    break;
                case SDL_KEYDOWN:
                    HandleKeyDown(e.key.keysym.scancode);
                    break;
                case SDL_MOUSEMOTION:
                    HandleMouseMotion(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
                    break;
                case SDL_MOUSEBUTTONUP:
                    HandleMouseUp(e.button.button);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    HandleMouseDown(e.button.button);
                    break;
                case SDL_WINDOWEVENT:
                    switch (e.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        case SDL_WINDOWEVENT_MAXIMIZED:
                            UpdateViewportSize();
                            break;
                        case SDL_WINDOWEVENT_RESTORED:
                            WindowRestored();
                            break;
                        case SDL_WINDOWEVENT_MINIMIZED:
                            WindowObscured();
                            break;
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            SetLowFPS(true);
                            break;
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                            SetLowFPS(false);
                            break;
                        default: break;
                    }
                    break;
                default:
                    break;
            }
        }
        ClearDepthOnly();

        ResetDPrintYPos();

        GlobalState *g = GetState();

#ifndef KEYBOARD_ROTATION
        SDL_SetRelativeMouseMode(g->currentState == MAIN_STATE ? SDL_TRUE : SDL_FALSE);
        // warp the mouse to the center of the screen if we are in the main game state
        if (g->currentState == MAIN_STATE)
        {
            const Vector2 realWndSize = ActualWindowSize();
            SDL_WarpMouseInWindow(GetGameWindow(), realWndSize.x / 2, realWndSize.y / 2);
        }
#endif

        if (g->UpdateGame) g->UpdateGame(g);

        g->cam->x = (float) g->level->position.x;
        g->cam->y = (float) g->CameraY;
        g->cam->z = (float) g->level->position.y;
        g->cam->yaw = (float) g->level->rotation;

        g->RenderGame(g);

        FrameGraphDraw();

        Swap();

        UpdateInputStates();

        if (g->requestExit)
        {
            quit = true;
        }

        FrameGraphUpdate(GetTimeNs() - frameStart);
        if (IsLowFPSModeEnabled()) SDL_Delay(33);
    }
    LogInfo("Mainloop exited, cleaning up engine...\n");
    DestroyGlobalState();
    SDL_DestroyWindow(GetGameWindow());
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
