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

SDL_Texture *pauseTexture;

char pauseOptions[3][32] = {
        "Resume",
        "Exit Level",
        "Quit"
};

int pauseSelected = 0;

void GPauseStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        PlaySoundEffect(gzwav_sfx_popdown);
        SDL_DestroyTexture(pauseTexture);
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
                SDL_DestroyTexture(pauseTexture);
                GMainStateSet();
                break;
            case 1:
                PlaySoundEffect(gzwav_sfx_popdown);
                SDL_DestroyTexture(pauseTexture);
                ChangeLevelByID(PAUSE_EXIT_LEVEL);
                GMainStateSet();
                break;
            case 2:
                PlaySoundEffect(gzwav_sfx_popdown);
                SDL_DestroyTexture(pauseTexture);
                GMenuStateSet();
                break;
        }
    }
}

void GPauseStateRender() {
    SDL_RenderCopy(GetRenderer(), pauseTexture, NULL, NULL);

    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    setColorUint(0x80000000);
    draw_rect(0, 0, WindowWidth(), WindowHeight());
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);

    DrawTextAligned("Game Paused", 32, 0xFFFFFFFF, vec2s(0), vec2(WindowWidth(), 300), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);

    char *levelID = gLevelEntries[GetState()->levelID].displayName;
    DrawTextAligned(levelID, 32, 0xFFFFFFFF, vec2(0,  200), vec2(WindowWidth(), 40), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);

    for (int i = 0; i < 3; i++) {
        if (i == pauseSelected) {
            DrawTextAligned(pauseOptions[i], 24, 0xFF00FF00, vec2(0,  300 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
        } else {
            DrawTextAligned(pauseOptions[i], 24, 0xFFFFFFFF, vec2(0,  300 + (30 * (i + 1))), vec2(WindowWidth(), 30), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
        }
    }
}

void GPauseStateSet() {
    PlaySoundEffect(gzwav_sfx_popup);
    pauseTexture = GetScreenshot();
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
    pauseSelected = 0;
}
