//
// Created by droc101 on 7/4/2024.
//

#include "GLevelSelectState.h"
#include <stdio.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMainState.h"
#include "GMenuState.h"

int GLevelSelectState_SelectedLevel = 0;

void GLevelSelectStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		GMenuStateSet();
	}
	if (IsKeyJustPressed(SDL_SCANCODE_DOWN) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
	{
		GLevelSelectState_SelectedLevel--;
		GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, GetLevelDataTable()->levelCount);
	} else if (IsKeyJustPressed(SDL_SCANCODE_UP) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
	{
		GLevelSelectState_SelectedLevel++;
		GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, GetLevelDataTable()->levelCount);
	} else if (IsKeyJustReleased(SDL_SCANCODE_SPACE) || IsButtonJustReleased(CONTROLLER_OK))
	{
		ConsumeKey(SDL_SCANCODE_SPACE);
		ConsumeButton(CONTROLLER_OK);
		ChangeLevelByID(GLevelSelectState_SelectedLevel);
		GMainStateSet();
	}
}

void GLevelSelectStateRender(GlobalState * /*State*/)
{
	SetColorUint(0xFF123456);
	ClearColor(0xFF123456);

	RenderMenuBackground();

	FontDrawString(v2(20, 20), GAME_TITLE, 128, 0xFFFFFFFF, false);
	FontDrawString(v2(20, 150), "Press Space to start.", 32, 0xFFa0a0a0, false);

	char *levelName = GetLevelDataTable()->levelEntries[GLevelSelectState_SelectedLevel].internalName;
	char levelNameBuffer[64];
	sprintf(levelNameBuffer, "%02d %s", GLevelSelectState_SelectedLevel + 1, levelName);
	DrawTextAligned(levelNameBuffer,
					32,
					0xFFFFFFFF,
					v2(50, 300),
					v2(WindowWidth() - 50, 300),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					false);
}

void GLevelSelectStateSet()
{
	StopMusic();
	SetStateCallbacks(GLevelSelectStateUpdate,
					  NULL,
					  LEVEL_SELECT_STATE,
					  GLevelSelectStateRender); // Fixed update is not needed for this state
}
