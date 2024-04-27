//
// Created by droc101 on 4/21/2024.
//
#include <math.h>
#include "../defines.h"
#include "Vector2.h"
#include "Wall.h"
#include "Level.h"
#include "../Helpers/Drawing.h"
#include "Ray.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/Error.h"
#include "Actor.h"

Level *CreateLevel() {
    Level *l = (Level*)malloc(sizeof(Level));
    l->actors = CreateList();
    l->walls = CreateList();
    l->position = vec2s(0);
    l->rotation = 0;
    l->SkyColor = 0xff82c5ff;
    l->FloorColor = 0xff36322f;
    return l;
}

void DestroyLevel(Level *l) {
    for (int i = 0; i < l->walls->size; i++) {
        Wall *w = (Wall *) ListGet(l->walls, i);
        FreeWall(w);
    }
    for (int i = 0; i < l->actors->size; i++) {
        Actor *a = (Actor *) ListGet(l->actors, i);
        FreeActor(a);
    }
    ListFreeWithData(l->walls);
    ListFree(l->actors); // actors are freed above (FreeActor)
    free(l);
}

double DepthBuffer[8192]; // if you have a screen wider than 8192 pixels, you're on your own

void RenderCol(Level *l, int col) {
    setColorUint(0xFFFFFFFF);
    double angle = atan2(col - WindowWidth() / 2, WindowWidth() / 2) + l->rotation;

    RayCastResult raycast = HitscanLevel(*l, l->position, angle, true, false, false); // scan walls only

    if (!raycast.Collided) {
        DepthBuffer[col] = 999999; // no wall hit, set to unreasonably far away
        return; // nothing else to do
    }

    double distance = Vector2Distance(l->position, raycast.CollisionPoint) * cos(angle - l->rotation);

    if (distance == 0) {
        distance = 0.000001;
        Error("Distance to wall is 0");
    }

    DepthBuffer[col] = distance; // store the distance for later (Actor pass)

    double height = WindowHeight() / distance;
    int y = (WindowHeight() - height) / 2;

    double shade = fabs(cos((l->rotation + (1.5 * PI)) - WallGetAngle(raycast.CollisionWall)));
    shade *= (1 - (distance / (WindowWidth() / 2)));
    shade = max(0.6, min(1, shade));
    //shade = floor(shade * 16) / 16;

    byte shadeByte = 255 * shade;


    SDL_Texture *texture = raycast.CollisionWall.tex;
    SDL_Point texSize = SDL_TextureSize(texture);
    uint texW = texSize.x;

    double wallLength = WallGetLength(raycast.CollisionWall);
    double localX = Vector2Distance(raycast.CollisionWall.a, raycast.CollisionPoint);
    double texCol = (localX / WallGetLength(raycast.CollisionWall)) * texW;

    texCol *= (wallLength / 2);
    texCol = fmod(texCol, texW);

    texCol = wrap(texCol, 0, texW - 1);

    SDL_SetTextureColorMod(texture, shadeByte, shadeByte, shadeByte);
    DrawTextureColumn(texture, texCol, col, y, height);
}

void RenderActorCol(Level *l, int col) {
    double angle = atan2(col - WindowWidth() / 2, WindowWidth() / 2) + l->rotation;

    RayCastResult raycast = HitscanLevel(*l, l->position, angle, false, true, true); // scan actors only

    if (!raycast.Collided) {
        return; // nothing else to do
    }

    double distance = Vector2Distance(l->position, raycast.CollisionPoint) * cos(angle - l->rotation);

    if (distance == 0) {
        distance = 0.000001;
    }

    if (distance > DepthBuffer[col]) {
        return; // Actor is behind a wall
    }

    double height = WindowHeight() / distance;
    int y = (WindowHeight() - height) / 2;

    double shade = fabs(cos((l->rotation + (1.5 * PI)) - WallGetAngle(raycast.CollisionWall)));
    shade *= (1 - (distance / (WindowWidth() / 2)));
    shade = max(0.6, min(1, shade));
    //shade = floor(shade * 16) / 16;

    byte shadeByte = 255 * shade;

    SDL_Texture *texture = raycast.CollisionWall.tex;
    SDL_Point texSize = SDL_TextureSize(texture);
    uint texW = texSize.x;

    double wallLength = WallGetLength(raycast.CollisionWall);
    double localX = Vector2Distance(raycast.CollisionWall.a, raycast.CollisionPoint);
    double texCol = (localX / WallGetLength(raycast.CollisionWall)) * texW;

    texCol *= (wallLength / 2);
    texCol = fmod(texCol, texW);

    texCol = wrap(texCol, 0, texW - 1);

    SDL_SetTextureColorMod(texture, shadeByte, shadeByte, shadeByte);
    DrawTextureColumn(texture, texCol, col, y, height);
}
