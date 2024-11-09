//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include <stdio.h>
#include "GLevelSelectState.h"
#include "GMainState.h"
#include "../Assets/Assets.h"
#include "../Helpers/LevelEntries.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/UI/Controls/Button.h"

UiStack *pauseStack = NULL;

void GPauseStateUpdate(GlobalState */*State*/)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
    {
        PlaySoundEffect(gzwav_sfx_popdown);
        GMainStateSet();
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GPauseStateRender(GlobalState *State)
{
    RenderLevel(State);

    SetColorUint(0x80000000);
    DrawRect(0, 0, WindowWidth(), WindowHeight());

    DrawTextAligned("Game Paused", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 250), FONT_HALIGN_CENTER,
                    FONT_VALIGN_MIDDLE, false);

    const char *levelID = gLevelEntries[State->levelID].displayName;
    const int cNum = gLevelEntries[State->levelID].courseNum;

    if (cNum != -1)
    {
        char buf[64];
        sprintf(buf, "Level %d", cNum);
        DrawTextAligned(buf, 16, 0xFF000000, v2(4, 164), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                        true);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, v2(0, 160), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                        true);
    }

    DrawTextAligned(levelID, 32, 0xFF000000, v2(4, 204), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    true);
    DrawTextAligned(levelID, 32, 0xFFFFFFFF, v2(0, 200), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    true);

    ProcessUiStack(pauseStack);
    DrawUiStack(pauseStack);
}

void BtnPauseResume()
{
    GMainStateSet();
}

void BtnPauseExit()
{
#ifdef USE_LEVEL_SELECT
    GLevelSelectStateSet();
#else
    ChangeLevelByID(PAUSE_EXIT_LEVEL);
#endif
}

void GPauseStateSet()
{
    if (pauseStack == NULL)
    {
        pauseStack = CreateUiStack();
        UiStackPush(pauseStack, CreateButtonControl(v2(0,20), v2(300, 40), "Resume", BtnPauseResume, MIDDLE_CENTER));
        UiStackPush(pauseStack, CreateButtonControl(v2(0,70), v2(300, 40), "Exit Level", BtnPauseExit, MIDDLE_CENTER));
    }
    UiStackResetFocus(pauseStack);

    PlaySoundEffect(gzwav_sfx_popup);
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate, NULL, PAUSE_STATE); // Fixed update is not needed for this state
}
