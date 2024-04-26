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
#include "GMainState.h"

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
        GMainStateSet();
    }
}

void GMenuStateRender() {
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

    FontDrawString(vec2(20, 20), "GAME.", 128, 0xFFFFFFFF);
    FontDrawString(vec2(20, 150), "Press Space to start.", 32, 0xFFa0a0a0);

#ifdef GMENUSTATE_WALL_DEBUG
    FontDrawString(vec2(10, 10), "wasd to move test player, mouse to look, space to start", 16, 0xFFFFFFFF);

    setColorUint(0xFFFFFFFF);
    SDL_RenderDrawLine(GetRenderer(), tWall->a.x, tWall->a.y, tWall->b.x, tWall->b.y);

    setColorUint(0xFF00FF00);
    draw_rect(tPlayerPos.x - 5, tPlayerPos.y - 5, 10, 10);

    RayCastResult rc = Intersect(*tWall, tPlayerPos, tPlayerRot);
    if (rc.Collided) {
        setColorUint(0xFFFF00FF);
        draw_rect(rc.CollisonPoint.x - 5, rc.CollisonPoint.y - 5, 10, 10);

        Vector2 PushedPos = PushPointOutOfWallHitbox(*tWall, vec2o(rc.CollisonPoint.x, rc.CollisonPoint.y, tPlayerPos.x, tPlayerPos.y));
        setColorUint(0xFF000000);
        draw_rect(PushedPos.x - 5, PushedPos.y - 5, 10, 10);

        char buffer[256];
        sprintf(buffer, "Collision Point: %f, %f\nPushed Point: %f, %f", rc.CollisonPoint.x, rc.CollisonPoint.y, PushedPos.x, PushedPos.y, 0xFF00FF00);
        FontDrawString(vec2(20, 200), buffer, 16, 0xFF00FF00);

        setColorUint(0xFF0000FF);
        SDL_RenderDrawLine(GetRenderer(), tPlayerPos.x, tPlayerPos.y, rc.CollisonPoint.x, rc.CollisonPoint.y);
    } else {
        FontDrawString(vec2(20, 200), "No Collision", 16, 0xFF808080);
        Vector2 dir = vec2(cos(tPlayerRot), sin(tPlayerRot));
        dir = Vector2Scale(dir, 10000);
        dir = Vector2Add(tPlayerPos, dir);
        setColorUint(0xFF0000FF);
        SDL_RenderDrawLine(GetRenderer(), tPlayerPos.x, tPlayerPos.y, dir.x, dir.y);
    }
#endif
}

void GMenuStateSet() {
#ifdef GMENUSTATE_WALL_DEBUG
    tWall = CreateWall(vec2(300, 400), vec2(600, 650), 0);
    tPlayerPos = vec2(400, 600);
#endif
    SetRenderCallback(GMenuStateRender);
    SetUpdateCallback(GMenuStateUpdate);
}
