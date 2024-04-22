//
// Created by droc101 on 4/22/2024.
//

#include "GPauseState.h"
#include "../input.h"
#include <math.h>
#include "../Structs/ray.h"
#include "../error.h"
#include "../Helpers/mathex.h"
#include "../Helpers/drawing.h"
#include "../Helpers/font.h"
#include <stdio.h>
#include "../Structs/GlobalState.h"
#include "GMainState.h"

SDL_Texture *pauseTexture;

void GPauseStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_RETURN)) {
        SDL_DestroyTexture(pauseTexture); // free the screenshot texture (we don't want to leak memory)
        // change to the main game state
        GMainStateSet();
    }
}

void GPauseStateRender() {
    SDL_RenderCopy(GetRenderer(), pauseTexture, NULL, NULL);

    FontDrawString(vec2(20, 150), "Game Paused\nPress enter to resume", 32);

}

void GPauseStateSet() {
    pauseTexture = GetScreenshot();
    SetRenderCallback(GPauseStateRender);
    SetUpdateCallback(GPauseStateUpdate);
}
