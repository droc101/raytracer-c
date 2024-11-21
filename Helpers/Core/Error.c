//
// Created by droc101 on 4/21/2024.
//

#include "Error.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Logging.h"
#include "zlib.h"
#include "../CommonAssets.h"
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Options.h"
#include "../Graphics/Drawing.h"

SDL_MessageBoxColorScheme mbColorScheme;

_Noreturn void RestartProgram()
{
    SDL_Quit();
    char *args[] = {GetState()->executablePath, NULL};
    execv(GetState()->executablePath, args);
    exit(1);
}

_Noreturn void Error_Internal(char *error, const char *file, const int line, const char *function)
{
    char buf[256];
#ifndef NDEBUG
    sprintf(buf, "%s\n \n%s:%d (%s)", error, file, line, function);
#else
    sprintf(buf, "%s", error);
#endif

    LogError(buf);

    char finalMb[768];
    sprintf(finalMb,
            "Sorry, but the game has crashed.\n\n%s\n\nEngine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s",
            buf, VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION,
            SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);

    SDL_MessageBoxData mb;
    mb.message = finalMb;
    mb.title = "Error";

#ifdef NDEBUG
    const int btnc = 2;
#else
    const int btnc = 3;
#endif

    SDL_MessageBoxButtonData buttons[btnc];
    buttons[0].buttonid = 0;
    buttons[0].text = "Exit";
    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    buttons[1].buttonid = 1;
    buttons[1].text = "Restart";
    buttons[1].flags = 0;
#ifndef NDEBUG
    buttons[2].buttonid = 2;
    buttons[2].text = "Debug";
    buttons[2].flags = 0;
#endif

    mb.buttons = buttons;
    mb.numbuttons = btnc;

    mb.colorScheme = &mbColorScheme;

    mb.window = GetGameWindow();
    mb.flags = SDL_MESSAGEBOX_ERROR;

    int buttonid;
    SDL_ShowMessageBox(&mb, &buttonid);

    switch (buttonid)
    {
        case 0:
            exit(1);
        case 1:
            RestartProgram();
        case 2:
            fflush(stdout);

#ifdef WIN32
            *(volatile int *) 0 = 0; // die immediately
#else
            // emit sigtrap to allow debugger to catch the error
            raise(SIGTRAP);
#endif
            break;
        default:
            exit(1);
    }
    while (1) {}
}

_Noreturn void FriendlyError(const char *title, const char *description)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, description, NULL);
    exit(1);
}

_Noreturn void RenderInitError()
{
    LogError("Failed to initialize renderer");
    SDL_HideWindow(GetGameWindow());
    SDL_MessageBoxData mb;
    mb.title = "Failed to initialize renderer";
    if (GetState()->options.renderer == RENDERER_OPENGL)
    {
        mb.message =
                "Failed to start the OpenGL renderer.\nPlease make sure your graphics card and drivers support OpenGL 4.6.";
    } else if (GetState()->options.renderer == RENDERER_VULKAN)
    {
        mb.message =
                "Failed to start the Vulkan renderer.\nPlease make sure your graphics card and drivers support Vulkan 1.3.";
    }

    mb.numbuttons = 2;
    SDL_MessageBoxButtonData buttons[2];
    buttons[0].buttonid = 0;
    buttons[0].text = GetState()->options.renderer == RENDERER_OPENGL ? "Try Vulkan" : "Try OpenGL";
    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    buttons[1].buttonid = 1;
    buttons[1].text = "Exit";
    buttons[1].flags = 0;

    mb.colorScheme = &mbColorScheme;
    mb.buttons = buttons;
    mb.window = NULL;
    mb.flags = SDL_MESSAGEBOX_ERROR;

    int buttonid;
    SDL_ShowMessageBox(&mb, &buttonid);
    if (buttonid == 0)
    {
        GetState()->options.renderer = GetState()->options.renderer == RENDERER_OPENGL
                                           ? RENDERER_VULKAN
                                           : RENDERER_OPENGL;
        SaveOptions(&GetState()->options);
        RestartProgram();
    } // else
    exit(1);
}

void SignalHandler(const int sig)
{
    if (sig == SIGSEGV)
    {
        Error("Segmentation Fault");
    }
    if (sig == SIGFPE)
    {
        Error("Floating Point Exception");
    }
}

void ErrorHandlerInit()
{
    SDL_MessageBoxColor bg;
    bg.r = 25;
    bg.g = 25;
    bg.b = 25;

    SDL_MessageBoxColor text;
    text.r = 255;
    text.g = 255;
    text.b = 255;

    SDL_MessageBoxColor buttonBorder;
    buttonBorder.r = 40;
    buttonBorder.g = 40;
    buttonBorder.b = 40;

    SDL_MessageBoxColor buttonBg;
    buttonBg.r = 35;
    buttonBg.g = 35;
    buttonBg.b = 35;

    mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BACKGROUND] = bg;
    mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_TEXT] = text;
    mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] = buttonBorder;
    mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] = buttonBg;
    mbColorScheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] = text;

#ifdef NDEBUG
    signal(SIGSEGV, SignalHandler);
    signal(SIGFPE, SignalHandler);
#endif
}
