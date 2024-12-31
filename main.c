#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "Assets/AssetReader.h"
#include "Assets/Assets.h"
#include "config.h"
#include "Debug/DPrint.h"
#include "Debug/FrameBenchmark.h"
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
#include "Structs/Vector2.h"

SDL_Surface *windowIcon;

/**
 * Attempt to initialize the executable path
 * @param argc Program argument count
 * @param argv Program arguments
 */
void ExecPathInit(const int argc, char *argv[])
{
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
}

/**
 * Initialize the SDL library
 */
void InitSDL()
{
	SDL_SetHint(SDL_HINT_APP_NAME, GAME_TITLE);
#ifdef SDL_PLATFORM_LINUX
	SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland,x11"); // required to fix an nvidia bug with glCopyTexImage2D
#endif

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC))
	{
		LogError("SDL_Init Error: %s\n", SDL_GetError());
		Error("Failed to initialize SDL");
	}
}

/**
 * Initialize the audio system (SDL_mixer)
 */
void InitAudio()
{
	Mix_AllocateChannels(SFX_CHANNEL_COUNT);

	SDL_AudioSpec spec;
	spec.freq = 48000;
	spec.format = SDL_AUDIO_S16LE;
	spec.channels = 2;

	if (Mix_OpenAudio(0, &spec))
	{
		GetState()->isAudioStarted = true;
	}
	else
	{
		GetState()->isAudioStarted = false;
		LogError("Mix_OpenAudio Error: %s\n", SDL_GetError());
	}
}

/**
 * Initialize the game window and renderer
 */
void WindowAndRenderInit()
{
	const Uint32 rendererFlags = currentRenderer == RENDERER_OPENGL ? SDL_WINDOW_OPENGL : SDL_WINDOW_VULKAN;
	SDL_Window *window = SDL_CreateWindow(GAME_TITLE,
									 DEF_WIDTH,
									 DEF_HEIGHT,
									 rendererFlags | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		LogError("SDL_CreateWindow Error: %s\n", SDL_GetError());
		Error("Failed to create window.");
	}
	DwmDarkMode(window);
	SDL_SetWindowFullscreen(window, GetState()->options.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	SetGameWindow(window);
	UpdateViewportSize();

	if (!RenderInit())
	{
		RenderInitError();
	}

	SDL_SetWindowMinimumSize(window, MIN_WIDTH, MIN_HEIGHT);
	SDL_SetWindowMaximumSize(window, MAX_WIDTH, MAX_HEIGHT);

	windowIcon = ToSDLSurface(gztex_interface_icon);
	SDL_SetWindowIcon(window, windowIcon);
}

/**
 * Handle an SDL event
 * @param event The SDL event to handle
 * @param shouldQuit Whether the program should quit after handling the event
 */
void HandleEvent(const SDL_Event event, bool *shouldQuit)
{
	switch (event.type)
	{
		case SDL_EVENT_QUIT:
			*shouldQuit = true;
		break;
		case SDL_EVENT_KEY_UP:
			HandleKeyUp(event.key.scancode);
		break;
		case SDL_EVENT_KEY_DOWN:
			HandleKeyDown(event.key.scancode);
		break;
		case SDL_EVENT_MOUSE_MOTION:
			HandleMouseMotion(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
		break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			HandleMouseUp(event.button.button);
		break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			HandleMouseDown(event.button.button);
		break;
		case SDL_EVENT_WINDOW_RESIZED:
			UpdateViewportSize();
		break;
		case SDL_EVENT_GAMEPAD_ADDED:
			HandleControllerConnect();
		break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			HandleControllerDisconnect(event.cdevice.which);
		break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			HandleControllerButtonDown(event.gbutton.button);
		break;
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			HandleControllerButtonUp(event.gbutton.button);
		break;
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			HandleControllerAxis(event.gaxis.axis, event.gaxis.value);
		break;
		default:
			break;
	}
}

int main(const int argc, char *argv[])
{
	ErrorHandlerInit();
	LogInit();
	LogInfo("Build time: %s at %s\n", __DATE__, __TIME__);
	LogInfo("Version: %s\n", VERSION);
	LogInfo("Initializing Engine\n");

	ExecPathInit(argc, argv);

	InitSDL();

	PhysicsThreadInit();
	InitState();

	if (!RenderPreInit())
	{
		RenderInitError();
	}

	InitAudio();

	WindowAndRenderInit();

	InitCommonAssets();

	ChangeLevelByID(STARTING_LEVEL);

	GLogoSplashStateSet();

	InitTimers();

	LogInfo("Engine initialized, entering mainloop\n");

	SDL_Event event;
	bool shouldQuit = false;
	while (!shouldQuit)
	{
		while (GetState()->freezeEvents)
        {
            SDL_Delay(100);
        }
		const ulong frameStart = GetTimeNs();
#ifdef BENCHMARK_SYSTEM_ENABLE
		BenchFrameStart();
#endif

		while (SDL_PollEvent(&event) != 0)
		{
			HandleEvent(event, &shouldQuit);
		}
		ClearDepthOnly();

		ResetDPrintYPos();

		GlobalState *state = GetState();

		SDL_SetWindowRelativeMouseMode(GetGameWindow(), state->currentState == MAIN_STATE);
		// warp the mouse to the center of the screen if we are in the main game state
		if (state->currentState == MAIN_STATE)
		{
			const Vector2 realWndSize = ActualWindowSize();
			SDL_WarpMouseInWindow(GetGameWindow(), realWndSize.x / 2, realWndSize.y / 2);
		}

		if (state->UpdateGame)
		{
			state->UpdateGame(state);
		}

#ifdef BENCHMARK_SYSTEM_ENABLE
		if (IsKeyJustPressed(SDL_SCANCODE_F8))
		{
			BenchToggle();
		}
#endif

		state->cam->x = (float)state->level->player.pos.x;
		state->cam->y = (float)state->cameraY;
		state->cam->z = (float)state->level->player.pos.y;
		state->cam->yaw = (float)state->level->player.angle;

		state->RenderGame(state);

		FrameGraphDraw();

		Swap();

		UpdateInputStates();

		if (state->requestExit)
		{
			shouldQuit = true;
		}

#ifdef BENCHMARK_SYSTEM_ENABLE
		BenchFrameEnd();
#endif

		FrameGraphUpdate(GetTimeNs() - frameStart);
	}
	LogInfo("Mainloop exited, cleaning up engine...\n");
	PhysicsThreadTerminate();
	DestroyGlobalState();
	SDL_DestroyWindow(GetGameWindow());
	SDL_DestroySurface(windowIcon);
	DestroyCommonAssets();
	InvalidateAssetCache(); // Free all assets
	RenderDestroy();
	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	SDL_Quit();
	LogDestroy();
	return 0;
}

// Exporting these symbols tells GPU drivers to use the dedicated GPU on hybrid systems
#ifdef WIN32
__declspec(dllexport) uint NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif
