//
// Created by droc101 on 4/22/2024.
//

#include "GMenuState.h"
#include "../input.h"
#include "../Structs/ray.h"
#include "../Helpers/drawing.h"
#include "../Helpers/font.h"
#include "../Structs/GlobalState.h"
#include "GMainState.h"

void GMenuStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // change to the main game state
        GMainStateSet();
    }
}

void GMenuStateRender() {
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

    FontDrawString(vec2(20, 20), "GAME.", 128);
    FontDrawString(vec2(20, 150), "Press Space to start.", 32);

}

void GMenuStateSet() {
    SetRenderCallback(GMenuStateRender);
    SetUpdateCallback(GMenuStateUpdate);
}
