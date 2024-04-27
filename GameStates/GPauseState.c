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
        SDL_DestroyTexture(pauseTexture); // free the screenshot texture (we don't want to leak memory)
        // change to the main game state
        GMainStateSet();
    }
}

void GPauseStateRender() {
    SDL_RenderCopy(GetRenderer(), pauseTexture, NULL, NULL);
    FontDrawString(vec2(20, 150), "Game Paused\nPress escape to resume", 32, 0xFFFFFFFF);
}

void GPauseStateSet() {
    pauseTexture = GetScreenshot();
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
}
