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
#include "../Helpers/CollisionHelper.h"
#include "../Debug/DPrint.h"

#include "../GameStates/GPauseState.h"

SDL_Texture *skyTex;

// TODO: Clean this up and move it to Wall.c/h
bool IsNearWall(Wall wall, Vector2 position) {
    // if (nearWall(wall, position)) {
    //     printf("wall: {start: {x: %f, y: %f}, end: {x: %f, y: %f}} position: {x: %f, y: %f} nearWall: %d isWallStraight: %d inHitbox: %d\n", wall.a.x, wall.a.y, wall.b.x, wall.b.y, position.x, position.y, nearWall(wall, position), isWallStraight(wall), !notInHitbox(wall, position));
    //     fflush(stdout);
    // }
    return nearWall(wall, position) && (isWallStraight(wall) || !notInHitbox(wall, position));
}

void GMainStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GPauseStateSet();
    }

    Level *l = GetState()->level;

    //Vector2 oldPos = l->position;
    Vector2 moveVec = vec2(0, 0);
    if (IsKeyPressed(SDL_SCANCODE_W)) {
        moveVec.x += 1;
    } else if (IsKeyPressed(SDL_SCANCODE_S)) {
        moveVec.x -= 1;
    }

    if (IsKeyPressed(SDL_SCANCODE_Q)) {
        moveVec.y -= 1;
    } else if (IsKeyPressed(SDL_SCANCODE_E)) {
        moveVec.y += 1;
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
        double dx = w->b.x - w->a.x;
        double dy = w->b.y - w->a.y;
        int mult = (l->position.x - w->a.x) * (w->b.y - w->a.y) - (l->position.y - w->a.y) * (w->b.x - w->a.x) < 0 ? -1 : 1;
        double hitboxSize = mult * WALL_HITBOX_EXTENTS;
        Vector2 pos = Vector2Add(l->position, moveVec);
        Vector2 hitboxOffset = vec2(hitboxSize * dy / WallGetLength(*w), -hitboxSize * dy / WallGetLength(*w));
        if (
                (mult * ((pos.x - w->a.x - hitboxOffset.x) * (w->b.y - w->a.y) - (pos.y - w->a.y - hitboxOffset.y) * (w->b.x - w->a.x)) <= 0) &&
                (mult * ((pos.x - w->a.x - hitboxOffset.x) * hitboxOffset.y - (pos.y - w->a.y - hitboxOffset.y) * hitboxOffset.x) <= 0) &&
                (mult * ((pos.y - w->a.y - hitboxOffset.y) * hitboxOffset.x - (pos.x - w->a.x - hitboxOffset.x) * hitboxOffset.y) <= 0)
            ) {
            double dydx = dy / (dx ? dx : 1);
            double dxdy = dx / (dy ? dy : 1);
            double wallLength = WallGetLength(*w);

            double newX = hitboxSize * dy / wallLength + (dx == 0 ? w->a.x : dy == 0 ? pos.x : (pos.y - w->a.y + w->a.x * dydx + pos.x * dxdy) / (dydx + dxdy)) - l->position.x;
            double newY = -hitboxSize * dx / wallLength + (dx == 0 ? pos.y : dy == 0 ? w->a.y : (pos.x - w->a.x + w->a.y * dxdy + pos.y * dydx) / (dxdy + dydx)) - l->position.y;

            // printf("pos: {x: %f, y: %f} nearWall %d\n", pos.x, pos.y, IsNearWall(*w, pos));
            fflush(stdout);
            // Vector2 vec = Vector2Rotate(moveVec, -WallGetAngle(*w));
            // double newX = vec.x * fabs(sin(angle - WallGetAngle(*w)));
            // double newY = vec.y * fabs(cos(angle - WallGetAngle(*w)));
            printf("oldX: %f\toldY: %f\tnewX: %f\tnewY: %f\t", moveVec.x, moveVec.y, newX, newY);
            printf("dx: %f\tdy: %f\tmag: %f\n", fabs(moveVec.x - newX), fabs(moveVec.y - newY), Vector2Length(vec2(fabs(moveVec.x - newX), fabs(moveVec.y - newY))));
            // printf("angleRad: %f angleDeg: %f changedAngle: %f\n", angle, radToDeg(angle), radToDeg((angle - WallGetAngle(*w))));
            // printf("wallAngle: %f moveVec: {x: %f, y: %f} moveVecRot: {x: %f, y: %f}\n", WallGetAngle(*w), moveVec.x, moveVec.y, Vector2Rotate(moveVec, 0).x, Vector2Rotate(moveVec, 0).y);
            fflush(stdout);
            // moveVec = Vector2Rotate(vec2(newX, newY), WallGetAngle(*w));
            moveVec.x = newX;
            moveVec.y = newY;
            // l->position = pos;
        }
    }
    l->position = Vector2Add(l->position, moveVec);
    // l->position = Vector2Add(l->position, moveVec);
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

