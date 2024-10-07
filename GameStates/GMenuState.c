//
// Created by droc101 on 4/22/2024.
//

#include <stdio.h>
#include "GMenuState.h"
#include "../Helpers/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"
#include "../Structs/GlobalState.h"
#include "GLevelSelectState.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/TextBox.h"

void GMenuStateUpdate(GlobalState * State) {

    if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // change to the main game state
#ifdef USE_LEVEL_SELECT
        GLevelSelectStateSet();
#else
        GMainStateSet();
#endif
    }
}

void GMenuStateRender(GlobalState * State) {

    // sorry for the confusing variable names
    Vector2 bg_tile_size = v2(320, 240); // size on screen
    Vector2 bg_tex_size = texture_size(gztex_interface_menu_bg_tile); // actual size of the texture

    Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    // draw the logo
    SDL_Rect logoRect;
    logoRect.x = (WindowWidth() - 480) / 2;
    logoRect.y = 32;
    logoRect.w = 480;
    logoRect.h = 320;
    DrawTexture(v2(logoRect.x, logoRect.y), v2(logoRect.w, logoRect.h), gztex_interface_menu_logo);

    if (GetState()->frame % 60 < 30) {
        DrawTextAligned("Press Space", 32, 0xFFFFFFFF, v2(0, WindowHeight() - 150), v2(WindowWidth(), 32),
                        FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, false);
    }

#ifndef NDEBUG
    FontDrawString(v2(20, 200), "DEBUG BUILD", 16, 0xFF00FF00, true);
#endif

    // draw version and copyright info
    char buffer[256];
    sprintf(buffer, "Engine %s\n%s", VERSION, COPYRIGHT);
    DrawTextAligned(buffer, 16, 0xFF000000, v2(WindowWidth() - 208, WindowHeight() - 208), v2(200, 200), FONT_HALIGN_RIGHT, FONT_VALIGN_BOTTOM, true);
    DrawTextAligned(buffer, 16, 0xFFa0a0a0, v2(WindowWidth() - 210, WindowHeight() - 210), v2(200, 200), FONT_HALIGN_RIGHT, FONT_VALIGN_BOTTOM, true);
}

void GMenuStateSet() {
    SetRenderCallback(GMenuStateRender);
    SetUpdateCallback(GMenuStateUpdate, NULL, MENU_STATE); // Fixed update is not needed for this state
}
