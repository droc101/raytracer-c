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
#include "../Debug/DPrint.h"

#include "../GameStates/GPauseState.h"

SDL_Texture *skyTex;

// TODO: Clean this up and move it to Wall.c/h
bool IsNearWall(Wall wall, Vector2 position) {
    bool abx = (wall.a.x + WALL_HITBOX_EXTENTS <= position.x || wall.a.x - WALL_HITBOX_EXTENTS <= position.x) && (
                   wall.b.x + WALL_HITBOX_EXTENTS >= position.x || wall.b.x - WALL_HITBOX_EXTENTS >= position.x);
    bool bax = (wall.b.x + WALL_HITBOX_EXTENTS <= position.x || wall.b.x - WALL_HITBOX_EXTENTS <= position.x) && (
                   wall.a.x + WALL_HITBOX_EXTENTS >= position.x || wall.a.x - WALL_HITBOX_EXTENTS >= position.x);
    bool aby = (wall.a.y + WALL_HITBOX_EXTENTS <= position.y || wall.a.y - WALL_HITBOX_EXTENTS <= position.y) && (
                   wall.b.y + WALL_HITBOX_EXTENTS >= position.y || wall.b.y - WALL_HITBOX_EXTENTS >= position.y);
    bool bay = (wall.b.y + WALL_HITBOX_EXTENTS <= position.y || wall.b.y - WALL_HITBOX_EXTENTS <= position.y) && (
                   wall.a.y + WALL_HITBOX_EXTENTS >= position.y || wall.a.y - WALL_HITBOX_EXTENTS >= position.y);
    return ((abx && aby) || (abx && bay) || (bax && aby) || (bax && bay)) && !(
               wall.a.x != wall.b.x && wall.a.y != wall.b.y && (
                   fabs((position.x - 1) * ((wall.b.y - wall.a.y) / (wall.b.x - wall.a.x)) + 10 - position.y) >
                   WALL_HITBOX_EXTENTS && fabs(
                       (position.y - 10) * ((wall.b.x - wall.a.x) / (wall.b.y - wall.a.y)) + 1 - position.x) >
                   WALL_HITBOX_EXTENTS));
}

void GMainStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GPauseStateSet();
    }

    Level *l = GetState()->level;

    //Vector2 oldPos = l->position;
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

    /* TODO
     * - Check for Actor collisions too
     * - Fix it
     * - Move to a better place so that other functions can use it (such as Actor movement)
    */
    for (int i = 0; i < l->walls->size; i++) {
        Wall *w = ListGet(l->walls, i);
        Vector2 pos = Vector2Add(l->position, moveVec);
        double angle = atan2(moveVec.y, moveVec.x);
        if (IsNearWall(*w, pos)) {
            printf("oldX: %f oldY: %f newX: %f newY: %f\n", moveVec.x, moveVec.y, moveVec.x * cos(angle) * cos(WallGetAngle(*w)), moveVec.y * sin(angle) * sin(WallGetAngle(*w)));
            fflush(stdout);
            moveVec.x *= cos(angle) * cos(WallGetAngle(*w));
            moveVec.y *= sin(angle) * sin(WallGetAngle(*w));
        }
    }
    l->position = Vector2Add(l->position, moveVec);
    // RayCastResult moveCheck = HitscanLevel(*l, oldPos, angle, true, true, false); // scan walls and actors
    // if (moveCheck.Collided) {
    //     double distance = fabs(Vector2Distance(oldPos, moveCheck.CollisionPoint));
    //     if (distance <= WALL_HITBOX_EXTENTS) {
    //         // push 0.5 units out of the wall
    //         l->position = PushPointOutOfWallHitbox(moveCheck.CollisionWall,
    //                                                vec2o(moveCheck.CollisionPoint.x, moveCheck.CollisionPoint.y,
    //                                                      l->position.x, l->position.y));
    //     } else {
    //         l->position = moveVec; // not close enough to the wall to collide
    //     }
    // } else {
    //     l->position = moveVec; // no collision, move freely
    // }

    if (IsKeyPressed(SDL_SCANCODE_A)) {
        l->rotation -= ROT_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        l->rotation += ROT_SPEED;
    }

    if (IsKeyJustPressed(SDL_SCANCODE_C)) {
        Error("Manually triggered error.");
    }

    l->rotation = wrap(l->rotation, 0, 2 * PI);

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

    double skyPos = remap(l->rotation, 0, 2 * PI, 0, 256);
    skyPos = (int) skyPos % 256;

    for (int i = -WindowWidth(); i < WindowWidth() * 3; i += 1) {
        double tuSize = 256.0 / WindowWidth();
        double tu = i * tuSize + skyPos;
        SDL_Rect src = {fmod(tu, 256), 0, 1, 256};
        SDL_Rect dest = {i, 0, 1, WindowHeight() / 2};
        SDL_RenderCopy(GetRenderer(), skyTex, &src, &dest);
    }

    setColorUint(l->FloorColor);
    draw_rect(0, WindowHeight() / 2, WindowWidth(), WindowHeight() / 2);


    for (int col = 0; col < WindowWidth(); col++) {
        RenderCol(l, col);
        RenderActorCol(l, col);
    }
    DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)", 0xFFFFFFFF, false, l->position.x, l->position.y, l->rotation, radToDeg(l->rotation));
}

void GMainStateSet() {
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate);
}

void InitSkyTex() {
    skyTex = ToSDLTexture((const unsigned char *) gztex_level_sky, FILTER_LINEAR);
}

