//
// Created by droc101 on 11/23/2024.
//

#include "GInputOptionsState.h"
#include "../../Assets/Assets.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Helpers/Graphics/RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/CheckBox.h"
#include "../../Structs/UI/Controls/Slider.h"
#include "../../Structs/UI/UiStack.h"
#include "../GOptionsState.h"

UiStack *inputOptionsStack;

void BtnInputOptionsBack()
{
	GOptionsStateSet();
}

void GInputOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
	{
		BtnInputOptionsBack();
	}
}

void SldOptionsMouseSensitivity(const double value)
{
	GetState()->options.mouseSpeed = value;
}

void CbOptionsControllerMode(const bool value)
{
	GetState()->options.controllerMode = value;
}

void GInputOptionsStateRender(GlobalState * /*State*/)
{
	RenderMenuBackground();

	DrawTextAligned("Input Options",
					32,
					0xFFFFFFFF,
					v2s(0),
					v2(WindowWidth(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					false);

	ProcessUiStack(inputOptionsStack);
	DrawUiStack(inputOptionsStack);
}

void GInputOptionsStateSet()
{
	if (inputOptionsStack == NULL)
	{
		inputOptionsStack = CreateUiStack();
		int opY = 40;
		const int opSpacing = 25;

		UiStackPush(inputOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Camera Sensitivity",
										SldOptionsMouseSensitivity,
										TOP_CENTER,
										0.01,
										2.00,
										GetState()->options.mouseSpeed,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(inputOptionsStack,
					CreateCheckboxControl(v2(0, opY),
										  v2(480, 40),
										  "Controller Mode",
										  CbOptionsControllerMode,
										  TOP_CENTER,
										  GetState()->options.controllerMode));
		opY += opSpacing;


		UiStackPush(inputOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnInputOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(inputOptionsStack);

	SetRenderCallback(GInputOptionsStateRender);
	SetUpdateCallback(GInputOptionsStateUpdate, NULL, INPUT_OPTIONS_STATE); // Fixed update is not needed for this state
}
