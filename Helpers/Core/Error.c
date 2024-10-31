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

_Noreturn void Error_Internal(char* error, const char* file, int line, const char* function) {

#ifndef NDEBUG
    // emit sigtrap to allow debugger to catch the error
    raise(SIGTRAP);
#endif

    char buf[256];
    sprintf(buf, "Error: %s\n \n%s:%d (%s)", error, file, line, function);
    printf("%s", buf);

    char dbgInfoBuf[256];
    sprintf(dbgInfoBuf, "Engine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: %d.%d.%d\nZlib Version: %s", VERSION, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL, ZLIB_VERSION);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buf, NULL);
    exit(1);
}

_Noreturn void FriendlyError(char* title, char* description) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, description, NULL);
    exit(1);
}

void SignalHandler(int sig) {
    if (sig == SIGSEGV) {
        Error("Segmentation Fault");
    } else if (sig == SIGFPE) {
        Error("Floating Point Exception");
    }
}

void SetSignalHandler() {
#ifdef NDEBUG
    signal(SIGSEGV, SignalHandler);
    signal(SIGFPE, SignalHandler);
#endif
}
