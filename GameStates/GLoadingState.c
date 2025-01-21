//
// Created by droc101 on 1/20/25.
//

#include "GLoadingState.h"

#include "../Helpers/Core/Logging.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMainState.h"

char loadStateLevelname[32];

void GLoadingStateUpdate(GlobalState *) {}

void GLoadingStateRender(GlobalState *)
{
	ClearColor(0xFF000000);
	DrawTextAligned("LOADING", 16, -1, v2s(0), v2(WindowWidth(), WindowHeight()), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
}

void GLoadingStateFixedUpdate(GlobalState *state, double /*delta*/) {
	if (state->physicsFrame == 10)
	{
		if (ChangeLevelByName((char*)&loadStateLevelname))
		{
			GMainStateSet();
		} else
		{
			LogError("Failed to load level: %s\n", loadStateLevelname);
		}
	}
}

void GLoadingSelectStateSet(char *levelName)
{
	strncpy(loadStateLevelname, levelName, 32);
	StopMusic();
	SetStateCallbacks(GLoadingStateUpdate,
					  GLoadingStateFixedUpdate,
					  LOADING_STATE,
					  GLoadingStateRender);
}

