//
// Created by droc101 on 4/22/2024.
//

#include "GMenuState.h"
#include <stdio.h>
#include "../Assets/Assets.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "GLevelSelectState.h"
#include "GOptionsState.h"

UiStack *menuStack;

void StartGame()
{
#ifdef USE_LEVEL_SELECT
	GLevelSelectStateSet();
#else
	GMainStateSet();
#endif
}

void QuitGame()
{
	GetState()->requestExit = true;
}

void OpenOptions()
{
	GOptionsStateSet(false);
}

void GMenuStateUpdate(GlobalState * /*State*/) {}

void GMenuStateRender(GlobalState * /*State*/)
{
	RenderMenuBackground();
	RenderInGameMenuBackground();

	// draw the logo
	SDL_Rect logoRect;
	logoRect.x = (WindowWidth() - 480) / 2;
	logoRect.y = 32;
	logoRect.w = 480;
	logoRect.h = 320;
	DrawTexture(v2(logoRect.x, logoRect.y), v2(logoRect.w, logoRect.h), gztex_interface_menu_logo);

#ifdef BUILDSTYLE_DEBUG
	FontDrawString(v2(20, 200), "DEBUG BUILD", 16, 0xFF00FF00, true);
#endif

	// draw version and copyright info
	char buffer[256];
	sprintf(buffer, "Engine %s\n%s", VERSION, COPYRIGHT);
	DrawTextAligned(buffer,
					16,
					0xFF000000,
					v2(WindowWidth() - 208, WindowHeight() - 208),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					true);
	DrawTextAligned(buffer,
					16,
					0xFFa0a0a0,
					v2(WindowWidth() - 210, WindowHeight() - 210),
					v2(200, 200),
					FONT_HALIGN_RIGHT,
					FONT_VALIGN_BOTTOM,
					true);

	ProcessUiStack(menuStack);
	DrawUiStack(menuStack);
}

void GMenuStateSet()
{
	if (menuStack == NULL)
	{
		menuStack = CreateUiStack();
		int opY = 80;
		const int opSpacing = 50;

		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Start", StartGame, MIDDLE_CENTER));
		opY += opSpacing;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Options", OpenOptions, MIDDLE_CENTER));
		opY += opSpacing;
		UiStackPush(menuStack, CreateButtonControl(v2(0, opY), v2(480, 40), "Quit", QuitGame, MIDDLE_CENTER));
		opY += opSpacing;
	}
	UiStackResetFocus(menuStack);
	StopMusic();

	SetStateCallbacks(GMenuStateUpdate, NULL, MENU_STATE, GMenuStateRender); // Fixed update is not needed for this state
}
