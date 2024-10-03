//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include "../Helpers/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"
#include "../Structs/GlobalState.h"
#include "GMainState.h"
#include "GMenuState.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/LevelEntries.h"
#include "GLevelSelectState.h"
#include <stdio.h>

char pauseOptions[2][32] = {
        "Resume",
        "Exit Level"
};

int pauseSelected = 0;

void GPauseStateUpdate(GlobalState * State) {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        PlaySoundEffect(gzwav_sfx_popdown);
        GMainStateSet();
        return;
    }


    if (IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
        pauseSelected++;
        pauseSelected = wrap(pauseSelected, 0, 2);
    } else if (IsKeyJustPressed(SDL_SCANCODE_UP)) {
        pauseSelected--;
        pauseSelected = wrap(pauseSelected, 0, 2);
    } else if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        switch (pauseSelected) {
            case 0:
                PlaySoundEffect(gzwav_sfx_popdown);
                GMainStateSet();
                break;
            case 1:
                PlaySoundEffect(gzwav_sfx_popdown);
#ifndef USE_LEVEL_SELECT
                ChangeLevelByID(PAUSE_EXIT_LEVEL);
                GMainStateSet();
#else
                GLevelSelectStateSet();
#endif
                break;
        }
    }
}

void GPauseStateRender(GlobalState* State) {
    setColorUint(0x80000000);
    draw_rect(0, 0, WindowWidth(), WindowHeight());

    DrawTextAligned("Game Paused", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 250), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, false);

    char *levelID = gLevelEntries[State->levelID].displayName;
    int cNum = gLevelEntries[State->levelID].courseNum;

    if (cNum != -1) {
        char buf[64];
        sprintf(buf, "Level %d", cNum);
        DrawTextAligned(buf, 16, 0xFF000000, v2(4, 164), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, v2(0, 160), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
    }

    DrawTextAligned(levelID, 32, 0xFF000000, v2(4, 204), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
    DrawTextAligned(levelID, 32, 0xFFFFFFFF, v2(0, 200), v2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);

    for (int i = 0; i < 2; i++) {
        if (i == pauseSelected) {
            DrawTextAligned(pauseOptions[i], 24, 0xFF000000, v2(2, 322 + (30 * (i + 1))), v2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
            DrawTextAligned(pauseOptions[i], 24, 0xFFFFFFFF, v2(0, 320 + (30 * (i + 1))), v2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        } else {
            DrawTextAligned(pauseOptions[i], 24, 0xFF000000, v2(2, 322 + (30 * (i + 1))), v2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
            DrawTextAligned(pauseOptions[i], 24, 0x80a0a0a0, v2(0, 320 + (30 * (i + 1))), v2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        }
    }
}

void GPauseStateSet() {
    PlaySoundEffect(gzwav_sfx_popup);
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
    pauseSelected = 0;
}
