//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include <stdio.h>
#include "../Helpers/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"
#include "../Structs/GlobalState.h"
#include "GMainState.h"
#include "GMenuState.h"
#include "../Helpers/MathEx.h"
#include "../config.h"
#include "../Helpers/LevelEntries.h"
#include "GLevelSelectState.h"

char pauseOptions[3][32] = {
        "Resume",
        "Exit Level",
        "Quit"
};

int pauseSelected = 0;

void GPauseStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        PlaySoundEffect(gzwav_sfx_popdown);
        GMainStateSet();
        return;
    }


    if (IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
        pauseSelected++;
        pauseSelected = wrap(pauseSelected, 0, 3);
    } else if (IsKeyJustPressed(SDL_SCANCODE_UP)) {
        pauseSelected--;
        pauseSelected = wrap(pauseSelected, 0, 3);
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
            case 2:
                PlaySoundEffect(gzwav_sfx_popdown);
                GMenuStateSet();
                break;
        }
    }
}

void GPauseStateRender() {
    GlobalState *state = GetState();
    Level *l = state->level;

    RenderLevel(l->position, l->rotation, state->FakeHeight);

    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    setColorUint(0x80000000);
    draw_rect(0, 0, WindowWidth(), WindowHeight());
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);

    DrawTextAligned("Game Paused", 32, 0xFFFFFFFF, vec2s(0), vec2(WindowWidth(), 300), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, false);

    char *levelID = gLevelEntries[GetState()->levelID].displayName;
    DrawTextAligned(levelID, 32, 0xFF000000, vec2(4,  204), vec2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
    DrawTextAligned(levelID, 32, 0xFFFFFFFF, vec2(0,  200), vec2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);

    for (int i = 0; i < 3; i++) {
        if (i == pauseSelected) {
            DrawTextAligned(pauseOptions[i], 24, 0xFF000000, vec2(2,  322 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
            DrawTextAligned(pauseOptions[i], 24, 0xFFFFFFFF, vec2(0,  320 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        } else {
            DrawTextAligned(pauseOptions[i], 24, 0xFF000000, vec2(2,  322 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
            DrawTextAligned(pauseOptions[i], 24, 0x80a0a0a0, vec2(0,  320 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
        }
    }
}

void GPauseStateSet() {
    PlaySoundEffect(gzwav_sfx_popup);
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
    pauseSelected = 0;
}
