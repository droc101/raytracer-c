#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>
#include "Assets/AssetReader.h"
#include "Assets/Assets.h"
#include "config.h"
#include "Debug/DPrint.h"
#include "Debug/FrameGrapher.h"
#include "defines.h"
#include "GameStates/GLogoSplashState.h"
#include "Helpers/CommonAssets.h"
#include "Helpers/Core/Error.h"
#include "Helpers/Core/Input.h"
#include "Helpers/Core/Logging.h"
#include "Helpers/Core/PhysicsThread.h"
#include "Helpers/Core/Timing.h"
#include "Helpers/Graphics/Drawing.h"
#include "Helpers/Graphics/RenderingHelpers.h"
#include "Helpers/PlatformHelpers.h"
#include "Structs/GlobalState.h"

int main(const int argc, char *argv[])
{
	LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
	LogInfo("Version: %s\n", VERSION);
	LogInfo("Initializing Engine\n");

	ErrorHandlerInit();

	if (argc < 1)
	{
		// this should *never* happen, but let's be safe
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

	SDL_SetHint(SDL_HINT_APP_NAME, GAME_TITLE);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		LogError("SDL_Init Error: %s\n", SDL_GetError());
		Error("Failed to initialize SDL");
	}

	PhysicsThreadInit();
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
	SDL_Window *w = SDL_CreateWindow(GAME_TITLE,
									 SDL_WINDOWPOS_UNDEFINED,
									 SDL_WINDOWPOS_UNDEFINED,
									 DEF_WIDTH,
									 DEF_HEIGHT,
									 rendererFlags | SDL_WINDOW_RESIZABLE);
	if (w == NULL)
	{
		LogError("SDL_CreateWindow Error: %s\n", SDL_GetError());
		Error("Failed to create window.");
	}
	DwmDarkMode(w);
	SDL_SetWindowFullscreen(w, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SetGameWindow(w);
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

	GLogoSplashStateSet();

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
				case SDL_CONTROLLERDEVICEADDED:
					HandleControllerConnect();
					break;
				case SDL_CONTROLLERDEVICEREMOVED:
					HandleControllerDisconnect(e.cdevice.which);
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					HandleControllerButtonDown(e.cbutton.button);
					break;
				case SDL_CONTROLLERBUTTONUP:
					HandleControllerButtonUp(e.cbutton.button);
					break;
				case SDL_CONTROLLERAXISMOTION:
					HandleControllerAxis(e.caxis.axis, e.caxis.value);
					break;
				default:
					break;
			}
		}
		GlobalState *g = GetState();

		if (FrameStart() != VK_SUCCESS)
		{
			if (g->UpdateGame)
			{
				g->UpdateGame(g);
			}
			if (IsLowFPSModeEnabled())
			{
				SDL_Delay(33);
			}
			continue;
		}

		ClearDepthOnly();

		ResetDPrintYPos();

		SDL_SetRelativeMouseMode(g->currentState == MAIN_STATE ? SDL_TRUE : SDL_FALSE);
		// warp the mouse to the center of the screen if we are in the main game state
		if (g->currentState == MAIN_STATE)
		{
			const Vector2 realWndSize = ActualWindowSize();
			SDL_WarpMouseInWindow(GetGameWindow(), (int)realWndSize.x / 2, (int)realWndSize.y / 2);
		}

		if (g->UpdateGame)
		{
			g->UpdateGame(g);
		}

		g->cam->x = (float)g->level->player.pos.x;
		g->cam->y = (float)g->CameraY;
		g->cam->z = (float)g->level->player.pos.y;
		g->cam->yaw = (float)g->level->player.angle;

		g->RenderGame(g);

		FrameGraphDraw();

        FrameEnd();

		UpdateInputStates();

		if (g->requestExit)
		{
			quit = true;
		}

		if (IsLowFPSModeEnabled()) SDL_Delay(33);
		FrameGraphUpdate(GetTimeNs() - frameStart);
	}
	LogInfo("Mainloop exited, cleaning up engine...\n");
	PhysicsThreadTerminate();
	DestroyGlobalState();
	SDL_DestroyWindow(GetGameWindow());
	SDL_FreeSurface(icon);
	DestroyCommonAssets();
	InvalidateAssetCache(); // Free all assets
	RenderDestroy();
	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
	SDL_Quit();
	return 0;
}

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
#ifdef WIN32
__declspec(dllexport) uint NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif
