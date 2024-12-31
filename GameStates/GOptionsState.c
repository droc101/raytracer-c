//
// Created by droc101 on 10/27/24.
//

#include "GOptionsState.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/UiStack.h"
#include "GMenuState.h"
#include "GPauseState.h"
#include "Options/GInputOptionsState.h"
#include "Options/GSoundOptionsState.h"
#include "Options/GVideoOptionsState.h"

UiStack *optionsStack;
bool optionsStateInGame = false;

void BtnOptionsBack()
{
	if (optionsStateInGame)
	{
		GPauseStateSet();
	} else
	{
		GMenuStateSet();
	}
}

void GOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
	{
		BtnOptionsBack();
	}
}

void GOptionsStateRender(GlobalState *state)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Options",
					32,
					0xFFFFFFFF,
					v2s(0),
					v2(WindowWidth(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					false);

	ProcessUiStack(optionsStack);
	DrawUiStack(optionsStack);
}

void GOptionsStateSet(bool inGame)
{
	optionsStateInGame = inGame;
	if (optionsStack == NULL)
	{
		optionsStack = CreateUiStack();
		int opY = 80;
		const int opSpacing = 45;

		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Video Options", GVideoOptionsStateSet, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Sound Options", GSoundOptionsStateSet, TOP_CENTER));
		opY += opSpacing;
		UiStackPush(optionsStack,
					CreateButtonControl(v2(0, opY), v2(480, 40), "Input Options", GInputOptionsStateSet, TOP_CENTER));
		opY += opSpacing;

		UiStackPush(optionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(optionsStack);

	SetStateCallbacks(GOptionsStateUpdate, NULL, OPTIONS_STATE, GOptionsStateRender); // Fixed update is not needed for this state
}
