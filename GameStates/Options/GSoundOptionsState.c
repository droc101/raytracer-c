//
// Created by droc101 on 11/23/2024.
//

#include "GSoundOptionsState.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Helpers/Graphics/RenderingHelpers.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Level.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/Slider.h"
#include "../../Structs/UI/UiStack.h"
#include "../GOptionsState.h"

UiStack *soundOptionsStack;

void BtnSoundOptionsBack()
{
	GOptionsStateSet(optionsStateInGame);
}

void SldOptionsMasterVolume(const double value)
{
	GetState()->options.masterVolume = value;
	UpdateVolume();
}

void SldOptionsMusicVolume(const double value)
{
	GetState()->options.musicVolume = value;
	UpdateVolume();
}

void SldOptionsSfxVolume(const double value)
{
	GetState()->options.sfxVolume = value;
	UpdateVolume();
}

void GSoundOptionsStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
	{
		BtnSoundOptionsBack();
	}
}

void GSoundOptionsStateRender(GlobalState *state)
{
	if (optionsStateInGame)
	{
		RenderInGameMenuBackground();
	} else
	{
		RenderMenuBackground();
	}

	DrawTextAligned("Sound Options",
					32,
					0xFFFFFFFF,
					v2s(0),
					v2(WindowWidth(), 100),
					FONT_HALIGN_CENTER,
					FONT_VALIGN_MIDDLE,
					false);

	ProcessUiStack(soundOptionsStack);
	DrawUiStack(soundOptionsStack);
}

void GSoundOptionsStateSet()
{
	if (soundOptionsStack == NULL)
	{
		soundOptionsStack = CreateUiStack();
		int opY = 80;
		const int opSpacing = 45;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Master Volume",
										SldOptionsMasterVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.masterVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"Music Volume",
										SldOptionsMusicVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.musicVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;
		UiStackPush(soundOptionsStack,
					CreateSliderControl(v2(0, opY),
										v2(480, 40),
										"SFX Volume",
										SldOptionsSfxVolume,
										TOP_CENTER,
										0.0,
										1.0,
										GetState()->options.sfxVolume,
										0.01,
										0.1,
										SliderLabelPercent));
		opY += opSpacing;


		UiStackPush(soundOptionsStack,
					CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnSoundOptionsBack, BOTTOM_CENTER));
	}
	UiStackResetFocus(soundOptionsStack);

	SetStateCallbacks(GSoundOptionsStateUpdate,
					  NULL,
					  SOUND_OPTIONS_STATE,
					  GSoundOptionsStateRender); // Fixed update is not needed for this state
}
