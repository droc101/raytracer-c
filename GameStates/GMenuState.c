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

//#define GMENUSTATE_WALL_DEBUG

#ifdef GMENUSTATE_WALL_DEBUG
Wall *tWall;
Vector2 tPlayerPos;
double tPlayerRot;
#endif

void GMenuStateUpdate() {

#ifdef GMENUSTATE_WALL_DEBUG
    // get the current mouse position
    int x, y;
    SDL_GetMouseState(&x, &y);

    // make the player look at the mouse
    tPlayerRot = atan2(y - tPlayerPos.y, x - tPlayerPos.x);

    if (IsKeyPressed(SDL_SCANCODE_W)) {
        tPlayerPos.y -= 1;
    } else if (IsKeyPressed(SDL_SCANCODE_S)) {
        tPlayerPos.y += 1;
    }

    if (IsKeyPressed(SDL_SCANCODE_A)) {
        tPlayerPos.x -= 1;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        tPlayerPos.x += 1;
    }
#endif

    if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // change to the main game state
#ifdef USE_LEVEL_SELECT
        GLevelSelectStateSet();
#else
        GMainStateSet();
#endif
    }
}

void GMenuStateRender() {

    Vector2 bg_tile_size = vec2(320, 240);
    for (int x = 0; x < WindowWidth(); x += bg_tile_size.x) {
        for (int y = 0; y < WindowHeight(); y += bg_tile_size.y) {
            SDL_RenderCopy(GetRenderer(), menu_bg_tex, NULL, &(SDL_Rect){x, y, bg_tile_size.x, bg_tile_size.y});
        }
    }

    //RenderLevel(vec2(9.63, -3.15), 3.25, 0);

    // draw the logo
    SDL_Rect logoRect;
    logoRect.x = (WindowWidth() - 480) / 2;
    logoRect.y = 32;
    logoRect.w = 480;
    logoRect.h = 320;
    SDL_RenderCopy(GetRenderer(), menu_logo_tex, NULL, &logoRect);

    if (GetState()->frame % 60 < 30) {
        DrawTextAligned("Press Space", 32, 0xFFFFFFFF, vec2(0, WindowHeight() - 150), vec2(WindowWidth(), 32),
                        FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, false);
    }

#ifndef NDEBUG
    FontDrawString(vec2(20, 200), "DEBUG BUILD", 16, 0xFF00FF00, true);
#endif

    // draw version and copyright info
    char buffer[256];
    sprintf(buffer, "RayCaster Engine %s\n%s", VERSION, COPYRIGHT);
    DrawTextAligned(buffer, 16, 0xFF000000, vec2(WindowWidth() - 208, WindowHeight() - 208), vec2(200, 200), FONT_HALIGN_RIGHT, FONT_VALIGN_BOTTOM, true);
    DrawTextAligned(buffer, 16, 0xFFa0a0a0, vec2(WindowWidth() - 210, WindowHeight() - 210), vec2(200, 200), FONT_HALIGN_RIGHT, FONT_VALIGN_BOTTOM, true);
}

void GMenuStateSet() {
#ifdef GMENUSTATE_WALL_DEBUG
    tWall = CreateWall(vec2(300, 400), vec2(600, 650), 0);
    tPlayerPos = vec2(400, 600);
#endif
    SetRenderCallback(GMenuStateRender);
    SetUpdateCallback(GMenuStateUpdate);
}
