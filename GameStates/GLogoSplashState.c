//
// Created by droc101 on 7/29/2024.
//

#include "GLogoSplashState.h"
#include <stdio.h>
#include "../Helpers/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"
#include "../Structs/GlobalState.h"
#include "GLevelSelectState.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/TextBox.h"
#include "GMenuState.h"

void GLogoSplashStateUpdate(GlobalState * State) {

    if (State->frame == 20) {
        PlaySoundEffect(gzwav_sfx_coincling);
    }

    if (State->frame == 120) {
        GMenuStateSet();
    }
}

void GLogoSplashStateRender(GlobalState * State) {
    setColorUint(0x0);
    SDL_Renderer *renderer = GetRenderer();
    SDL_RenderClear(renderer);
    if (State->frame < 20 || State->frame > 100) {
        return;
    }

    // draw logo 300x300 centered
    SDL_Rect dest = {WindowWidth()/2 - 150, WindowHeight()/2 - 150, 300, 300};
    SDL_RenderCopy(renderer, studioLogoTex, NULL, &dest);
}

void GLogoSplashStateSet() {
    SetRenderCallback(GLogoSplashStateRender);
    SetUpdateCallback(GLogoSplashStateUpdate);
}

