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

SDL_Texture *pauseTexture;

void GPauseStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        PlaySoundEffect(gzwav_sfx_popdown);
        SDL_DestroyTexture(pauseTexture);
        GMainStateSet();
    }
}

void GPauseStateRender() {
    SDL_RenderCopy(GetRenderer(), pauseTexture, NULL, NULL);

    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    setColorUint(0x80000000);
    draw_rect(0, 0, WindowWidth(), WindowHeight());
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);

    DrawTextAligned("Game Paused\nPress escape to resume", 32, 0xFFFFFFFF, vec2s(0), vec2(WindowWidth(), 300), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
}

void GPauseStateSet() {
    PlaySoundEffect(gzwav_sfx_popup);
    pauseTexture = GetScreenshot();
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
}
