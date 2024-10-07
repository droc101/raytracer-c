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

uint GLogoSplashStateFixedUpdate(const uint interval, GlobalState* State) {

    if (State->frame == 20) {
        PlaySoundEffect(gzwav_sfx_coincling);
    }

    if (State->frame == 120) {
        GMenuStateSet();
    }

    State->frame++;
    return interval;
}

void GLogoSplashStateRender(GlobalState * State) {
    setColorUint(0x0);
    ClearColor(0xFF000000);
    if (State->frame < 20 || State->frame > 100) {
        return;
    }

    SDL_Rect dest = {WindowWidth()/2 - 150, WindowHeight()/2 - 150, 300, 300};
    DrawTexture(v2(dest.x, dest.y), v2(dest.w, dest.h), gztex_interface_studio);
}

void GLogoSplashStateSet() {
    SetRenderCallback(GLogoSplashStateRender);
    SetUpdateCallback(NULL, GLogoSplashStateFixedUpdate, LOGO_SPLASH_STATE); // Non-fixed is not needed for this state
}

