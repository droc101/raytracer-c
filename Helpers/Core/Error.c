//
// Created by droc101 on 4/21/2024.
//

#include "Error.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Options.h"
#include "../Graphics/Drawing.h"
#include "Logging.h"

SDL_MessageBoxColorScheme mbColorScheme;

_Noreturn inline void _GameAllocFailure()
{
	LogError("Memory Allocation Failed: %s\n", strerror(errno));
	if (errno == ENOMEM)
	{
		exit(1); // We should not attempt to do complex things if we are out of memory
	}
	Error("Memory Allocation Failed");
}

_Noreturn void RestartProgram()
{
	SDL_Quit();
	char *args[] = {GetState()->executablePath, NULL};
	execv(GetState()->executablePath, args);
	exit(1);
}

_Noreturn void _ErrorInternal(char *error, const char *file, const int line, const char *function)
{
	if (GetGameWindow() != NULL)
	{
		GetState()->freezeEvents = true;
		SDL_SetWindowRelativeMouseMode(GetGameWindow(), false);
	}

	char messageBuffer[256];
#ifdef BUILDSTYLE_DEBUG
	sprintf(messageBuffer, "%s\n \n%s:%d (%s)", error, file, line, function);
#else
	sprintf(messageBuffer, "%s", error);
#endif

	LogError(messageBuffer);

	char messageBoxTextBuffer[768];
	sprintf(messageBoxTextBuffer,
			"Sorry, but the game has crashed.\n\n%s\n\nEngine Version: %s\nSDL Version: %d.%d.%d\nSDL_Mixer Version: "
			"%d.%d.%d\nZlib Version: %s",
			messageBuffer,
			VERSION,
			SDL_MAJOR_VERSION,
			SDL_MINOR_VERSION,
			SDL_MICRO_VERSION,
			SDL_MIXER_MAJOR_VERSION,
			SDL_MIXER_MINOR_VERSION,
			SDL_MIXER_MICRO_VERSION,
			ZLIB_VERSION);

	SDL_MessageBoxData mb;
	mb.message = messageBoxTextBuffer;
	mb.title = "Error";

#ifdef BUILDSTYLE_RELEASE
	const int buttonCount = 2;
#else
	const int buttonCount = 3;
#endif

	SDL_MessageBoxButtonData buttons[buttonCount];
	buttons[0].buttonID = 0;
	buttons[0].text = "Exit";
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[1].buttonID = 1;
	buttons[1].text = "Restart";
	buttons[1].flags = 0;
#ifdef BUILDSTYLE_DEBUG
	buttons[2].buttonID = 2;
	buttons[2].text = "Debug";
	buttons[2].flags = 0;
#endif

	mb.buttons = buttons;
	mb.numbuttons = buttonCount;

	mb.colorScheme = &mbColorScheme;

	mb.window = GetGameWindow();
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID;
	SDL_ShowMessageBox(&mb, &pressedButtonID);

	switch (pressedButtonID)
	{
		case 0:
			exit(1);
		case 1:
			RestartProgram();
		case 2:
			fflush(stdout);

#ifdef WIN32
			*(volatile int *)0 = 0; // die immediately
#else
			// emit sigtrap to allow debugger to catch the error
			raise(SIGTRAP);
#endif
			break;
		default:
			exit(1);
	}
	while (true)
	{}
}

_Noreturn void FriendlyError(const char *title, const char *description)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, description, NULL);
	exit(1);
}

void PromptRelaunch(const char *title, const char *description, const char *yesBtn, const char *noBtn)
{
	SDL_MessageBoxData mb;
	mb.message = description;
	mb.title = title;

	SDL_MessageBoxButtonData buttons[2];
	buttons[0].buttonID = 0;
	buttons[0].text = noBtn;
	buttons[0].flags = 0;
	buttons[1].buttonID = 1;
	buttons[1].text = yesBtn;
	buttons[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;

	mb.buttons = buttons;
	mb.numbuttons = 2;

	mb.colorScheme = &mbColorScheme;

	mb.window = GetGameWindow();
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID;
	SDL_ShowMessageBox(&mb, &pressedButtonID);

	if (pressedButtonID == 1)
	{
		RestartProgram();
	}
}

_Noreturn void RenderInitError()
{
	LogError("Failed to initialize renderer");
	SDL_HideWindow(GetGameWindow());
	SDL_MessageBoxData mb;
	mb.title = "Failed to initialize renderer";
	if (GetState()->options.renderer == RENDERER_OPENGL)
	{
		mb.message = "Failed to start the OpenGL renderer.\n"
					 "Please make sure your graphics card and drivers support OpenGL 3.3.";
	} else if (GetState()->options.renderer == RENDERER_VULKAN)
	{
		mb.message = "Failed to start the Vulkan renderer.\n"
					 "Please make sure your graphics card and drivers support Vulkan 1.3.";
	}

	mb.numbuttons = 2;
	SDL_MessageBoxButtonData buttons[2];
	buttons[0].buttonID = 0;
	buttons[0].text = GetState()->options.renderer == RENDERER_OPENGL ? "Try Vulkan" : "Try OpenGL";
	buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	buttons[1].buttonID = 1;
	buttons[1].text = "Exit";
	buttons[1].flags = 0;

	mb.colorScheme = &mbColorScheme;
	mb.buttons = buttons;
	mb.window = NULL;
	mb.flags = SDL_MESSAGEBOX_ERROR;

	int pressedButtonID;
	SDL_ShowMessageBox(&mb, &pressedButtonID);
	if (pressedButtonID == 0)
	{
		if (GetState()->options.renderer == RENDERER_OPENGL)
		{
			GetState()->options.renderer = RENDERER_VULKAN;
		} else
		{
			GetState()->options.renderer = RENDERER_OPENGL;
		}
		SaveOptions(&GetState()->options);
		RestartProgram();
	} // else
	exit(1);
}

void SignalHandler(const int sig)
{
	switch (sig)
	{
		case SIGSEGV:
			Error("Segmentation Fault");
		case SIGFPE:
			Error("Floating Point Exception");
		default:
			break;
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

#ifdef BUILDSTYLE_RELEASE
	signal(SIGSEGV, SignalHandler);
	signal(SIGFPE, SignalHandler);
#endif
}
