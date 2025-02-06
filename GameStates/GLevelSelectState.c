//
// Created by droc101 on 7/4/2024.
//

#include "GLevelSelectState.h"
#include <dirent.h>
#include <stdio.h>

#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/Vector2.h"
#include "GLoadingState.h"
#include "GMainState.h"
#include "GMenuState.h"

int GLevelSelectState_SelectedLevel = 0;
List levelList;

void GLevelSelectStateUpdate(GlobalState * /*State*/)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(CONTROLLER_CANCEL))
	{
		GMenuStateSet();
	}
	if (levelList.length > 0)
	{
		if (IsKeyJustPressed(SDL_SCANCODE_DOWN) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
		{
			GLevelSelectState_SelectedLevel--;
			GLevelSelectState_SelectedLevel = (int)wrap(GLevelSelectState_SelectedLevel, 0, (double)levelList.length);
		} else if (IsKeyJustPressed(SDL_SCANCODE_UP) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
		{
			GLevelSelectState_SelectedLevel++;
			GLevelSelectState_SelectedLevel = (int)wrap(GLevelSelectState_SelectedLevel, 0, (double)levelList.length);
		} else if (IsKeyJustReleased(SDL_SCANCODE_SPACE) || IsButtonJustReleased(CONTROLLER_OK))
		{
			ConsumeKey(SDL_SCANCODE_SPACE);
			ConsumeButton(CONTROLLER_OK);
			ChangeLevelByName(ListGet(levelList, GLevelSelectState_SelectedLevel));
			GMainStateSet();
		}
	}
}

void GLevelSelectStateRender(GlobalState * /*State*/)
{
	SetColorUint(0xFF123456);
	ClearColor(0xFF123456);

	RenderMenuBackground();

	FontDrawString(v2(20, 20), GAME_TITLE, 128, 0xFFFFFFFF, largeFont);
	FontDrawString(v2(20, 150), "Press Space to start.", 32, 0xFFa0a0a0, largeFont);

	char levelNameBuffer[64];

	if (levelList.length > 0)
	{
		char *levelName = ListGet(levelList, GLevelSelectState_SelectedLevel);

		sprintf(levelNameBuffer, "%02d %s", GLevelSelectState_SelectedLevel + 1, levelName);
	} else
	{
		strcpy((char*)&levelNameBuffer, "No levels found");
	}

	DrawTextAligned(levelNameBuffer,
					32,
					0xFFFFFFFF,
					v2(50, 300),
					v2(WindowWidth() - 50, 300),
					FONT_HALIGN_LEFT,
					FONT_VALIGN_MIDDLE,
					largeFont);
}

void LoadLevelList()
{
	ListCreate(&levelList);
	char levelDataPath[300];
	sprintf(levelDataPath, "%sassets/level/", GetState()->executableFolder);

	// Get the name of all gmap files in the level directory
	DIR *dir = opendir(levelDataPath);
	if (dir == NULL)
	{
		LogError("Failed to open level directory: %s\n", levelDataPath);
		return;
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
	{
		if (strstr(ent->d_name, ".gmap") != NULL)
		{
			char *levelName = malloc(strlen(ent->d_name) + 1);
			CheckAlloc(levelName);
			strcpy(levelName, ent->d_name);
			// Remove the .gmap extension
			levelName[strlen(levelName) - 5] = '\0';
			ListAdd(&levelList, levelName);
		}
	}
	closedir(dir);
}

void GLevelSelectStateSet()
{
	if (levelList.length == 0)
	{
		LoadLevelList();
	}
	StopMusic();
	SetStateCallbacks(GLevelSelectStateUpdate,
					  NULL,
					  LEVEL_SELECT_STATE,
					  GLevelSelectStateRender); // Fixed update is not needed for this state
}

void GLevelSelectStateDestroy()
{
	ListFreeOnlyContents(levelList);
}
