//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <stdio.h>
#include <math.h>
#include "../Helpers/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Error.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"

#include "../GameStates/GPauseState.h"

SDL_Texture *skyTex;

void GMainStateUpdate() {

    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GPauseStateSet();
    }

    Level *l = GetState()->level;

    Vector2 oldPos = l->position;
    Vector2 moveVec = vec2(0, 0);
    if (IsKeyPressed(SDL_SCANCODE_W)) {
        moveVec.x += MOVE_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_S)) {
        moveVec.x -= MOVE_SPEED;
    }

    if (IsKeyPressed(SDL_SCANCODE_Q)) {
        moveVec.y -= MOVE_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_E)) {
        moveVec.y += MOVE_SPEED;
    }

    if (moveVec.x != 0 || moveVec.y != 0) {
        moveVec = Vector2Normalize(moveVec);
    }
    moveVec = Vector2Scale(moveVec, MOVE_SPEED);
    moveVec = Vector2Rotate(moveVec, l->rotation);
    moveVec = Vector2Add(l->position, moveVec);

    double angle = atan2(moveVec.y - oldPos.y, moveVec.x - oldPos.x);

    RayCastResult moveCheck = HitscanLevel(*l, oldPos, angle, true, true, false); // scan walls and actors
    if (moveCheck.Collided) {
        double distance = fabs(Vector2Distance(oldPos, moveCheck.CollisonPoint));
        if (distance <= WALL_HITBOX_EXTENTS) {
            // push 0.5 units out of the wall
            l->position = PushPointOutOfWallHitbox(moveCheck.CollisionWall,
                                                   vec2o(moveCheck.CollisonPoint.x, moveCheck.CollisonPoint.y,
                                                         l->position.x, l->position.y));
        } else {
            l->position = moveVec; // not close enough to the wall to collide
        }
    } else {
        l->position = moveVec; // no collision, move freely
    }

    if (IsKeyPressed(SDL_SCANCODE_A)) {
        l->rotation -= ROT_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        l->rotation += ROT_SPEED;
    }

    if (IsKeyJustPressed(SDL_SCANCODE_C)) {
        Error("Manually triggered error.");
    }

    l->rotation = wrap(l->rotation, 0, 2*PI);

    for (int i = 0; i < l->actors->size; i++) {
        Actor *a = (Actor *) ListGet(l->actors, i);
        a->Update(a);
    }
}

void GMainStateRender() {
    Level *l = GetState()->level;

    byte *sc = getColorUint(l->SkyColor);
    SDL_SetTextureColorMod(skyTex, sc[0], sc[1], sc[2]);
    free(sc);

    double skyPos = remap(l->rotation, 0, 2*PI, 0, 256);
    skyPos = (int)skyPos % 256;

    for (int i = -WindowWidth(); i < WindowWidth() * 3; i += 1) {
        double tuSize = 256.0 / WindowWidth();
        double tu = (i * tuSize) + skyPos;
        SDL_Rect src = {fmod(tu, 256), 0, 1, 256};
        SDL_Rect dest = {i, 0, 1, WindowHeight()/2};
        SDL_RenderCopy(GetRenderer(), skyTex, &src, &dest);
    }

    //SDL_RenderClear(GetRenderer());

    setColorUint(l->FloorColor);
    draw_rect(0, WindowHeight()/2, WindowWidth(), WindowHeight()/2);


    for (int col = 0; col < WindowWidth(); col++) {
        RenderCol(l, col);
        RenderActorCol(l, col);
    }

    char buffer[64];
    sprintf(buffer, "Position %.2f, %.2f\nRotation %.4f", l->position.x, l->position.y, l->rotation);
    FontDrawString(vec2(20, 20), buffer, 16, 0xFFFFFFFF);

    sprintf(buffer, "HP %d\nAmmo %d", GetState()->hp, GetState()->ammo);
    //FontDrawString(vec2(20, HEIGHT - 20 - (24*2)), buffer, 24);
}

void GMainStateSet() {
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate);
}

void InitSkyTex() {
    skyTex = ToSDLTexture((const unsigned char *) tex_level_sky, FILTER_LINEAR);
}

