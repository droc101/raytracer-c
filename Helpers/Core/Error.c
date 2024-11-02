//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "../../defines.h"
#include "Error.h"
#include "../Graphics/Drawing.h"
#include "../Graphics/Font.h"
#include <string.h>
#include <signal.h>
#include "zlib.h"
#include "../CommonAssets.h"
#include "../../Structs/GlobalState.h"

_Noreturn void Error_Internal(char *error, const char *file, const int line, const char *function)
{
    char buf[256];
    sprintf(buf, "Error: %s\n \n%s:%d (%s)", error, file, line, function);
    printf("%s", buf);

#ifndef NDEBUG

    fflush(stdout);

#ifdef WIN32
    *(volatile int*)0 = 0; // die immediately
#else
    // emit sigtrap to allow debugger to catch the error
    raise(SIGTRAP);
#endif

#endif

    char finalMb[768];
    sprintf(finalMb,
            "Sorry, but the game has crashed.\n\n%s\n\nEngine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s",
            buf, VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION,
            SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);

    SDL_MessageBoxData mb;
    mb.message = finalMb;
    mb.title = "Error";

    SDL_MessageBoxButtonData buttons[2];
    buttons[0].buttonid = 0;
    buttons[0].text = "Exit";
    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    buttons[1].buttonid = 1;
    buttons[1].text = "Restart";
    buttons[1].flags = 0;

    mb.buttons = buttons;
    mb.numbuttons = 2;

    mb.window = GetWindow();
    mb.flags = SDL_MESSAGEBOX_ERROR;

    int buttonid;
    SDL_ShowMessageBox(&mb, &buttonid);

    if (buttonid == 0)
    {
        exit(1);
    } else
    {
        // restart
        SDL_Quit();
        char *args[] = {GetState()->executablePath, NULL};
        execv(GetState()->executablePath, args);
        exit(1);
    }
}

_Noreturn void FriendlyError(const char *title, const char *description)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, description, NULL);
    exit(1);
}

void SignalHandler(const int sig)
{
    if (sig == SIGSEGV)
    {
        Error("Segmentation Fault");
    } else if (sig == SIGFPE)
    {
        Error("Floating Point Exception");
    }
}

void SetSignalHandler()
{
#ifdef NDEBUG
    signal(SIGSEGV, SignalHandler);
    signal(SIGFPE, SignalHandler);
#endif
}
