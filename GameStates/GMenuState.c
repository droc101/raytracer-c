//
// Created by droc101 on 4/22/2024.
//

#include <stdio.h>
#include "GMenuState.h"
#include "../input.h"
#include "../Structs/ray.h"
#include "../Helpers/drawing.h"
#include "../Helpers/font.h"
#include "../Structs/GlobalState.h"
#include "GMainState.h"

#include "../Structs/ray.h"

Wall *tWall;
Vector2 tPlayerPos;
double tPlayerRot;

void GMenuStateUpdate() {

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

    if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // change to the main game state
        GMainStateSet();
    }
}



void GMenuStateRender() {
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

    //FontDrawString(vec2(20, 20), "GAME.", 128);
    //FontDrawString(vec2(20, 150), "Press Space to start.", 32);

    FontDrawString(vec2(10, 10), "wasd to move test player, mouse to look, space to start", 16);

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
        sprintf(buffer, "Collision Point: %f, %f\nPushed Point: %f, %f", rc.CollisonPoint.x, rc.CollisonPoint.y, PushedPos.x, PushedPos.y);
        FontDrawString(vec2(20, 200), buffer, 16);

        setColorUint(0xFF0000FF);
        SDL_RenderDrawLine(GetRenderer(), tPlayerPos.x, tPlayerPos.y, rc.CollisonPoint.x, rc.CollisonPoint.y);
    } else {
        FontDrawString(vec2(20, 200), "No Collision", 16);
        Vector2 dir = vec2(cos(tPlayerRot), sin(tPlayerRot));
        dir = Vector2Scale(dir, 1024);
        dir = Vector2Add(tPlayerPos, dir);
        setColorUint(0xFF0000FF);
        SDL_RenderDrawLine(GetRenderer(), tPlayerPos.x, tPlayerPos.y, dir.x, dir.y);
    }





}

void GMenuStateSet() {
    tWall = CreateWall(vec2(300, 400), vec2(600, 650), 0);
    tPlayerPos = vec2(400, 600);
    tPlayerRot = -(PI/4);
    SetRenderCallback(GMenuStateRender);
    SetUpdateCallback(GMenuStateUpdate);
}
