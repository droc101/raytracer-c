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
#include "GlobalState.h"
#include "../Helpers/List.h"

SDL_Texture *skyTex;

Level *CreateLevel() {
    Level *l = (Level*)malloc(sizeof(Level));
    l->actors = CreateList();
    l->walls = CreateList();
    l->position = vec2s(0);
    l->rotation = 0;
    l->SkyColor = 0xff82c5ff;
    l->FloorColor = 0xff36322f;
    l->MusicID = -1;
    l->FogColor = 0xff000000;
    l->FogStart = 10;
    l->FogEnd = 30;
    l->staticWalls = NULLPTR;
    l->staticActors = NULLPTR;
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

    if (l->staticWalls != NULL) {
        DestroySizedArray(l->staticWalls);
    }

    if (l->staticActors != NULL) {
        DestroySizedArray(l->staticActors);
    }

    ListFreeWithData(l->walls);
    ListFree(l->actors); // actors are freed above (FreeActor)
    free(l);
}

void BakeWallArray(Level *l) {
    l->staticWalls = ToSizedArray(l->walls);
}

void BakeActorArray(Level *l) {
    l->staticActors = ToSizedArray(l->actors);
}

double DepthBuffer[8192]; // if you have a screen wider than 8192 pixels, you're on your own

void RenderCol(Level *l, int col, Vector2 position, double rotation) {

    //setColorUint(0xFFFFFFFF);
    double angle = atan2(col - WindowWidth() / 2, WindowWidth() / 2) + rotation;

    RayCastResult raycast = HitscanLevel(*l, position, angle, true, false, false); // scan walls only

    if (!raycast.Collided) {
        DepthBuffer[col] = 999999; // no wall hit, set to unreasonably far away
        return; // nothing else to do
    }

    double distance = raycast.Distance * cos(angle - l->rotation);

    if (distance == 0) {
        distance = 0.000001;
        Error("Distance to wall is 0");
    }

    DepthBuffer[col] = distance; // store the distance for later (Actor pass)

    double height = WindowHeight() / distance;
    int y = (WindowHeight() - height) / 2;

    y += (GetState()->FakeHeight / distance);

    double shade = fabs(cos((l->rotation + (1.5 * PI)) - raycast.CollisionWall.Angle));
    shade *= (1 - (distance / (WindowWidth() / 2)));
    shade = max(0.6, min(1, shade));
    //shade = floor(shade * 16) / 16;

    // calculate the fog color
    double fogFactor = remap(distance, l->FogStart, l->FogEnd, 0, 1);
    fogFactor = max(0, min(1, fogFactor));
//    fogFactor = floor(fogFactor * 16) / 16;
    uint fogColor = l->FogColor;
    fogColor = (fogColor & 0x00FFFFFF) | ((uint)(fogFactor * 255) << 24);

    byte shadeByte = 255 * shade;

    SDL_Texture *texture = raycast.CollisionWall.tex;
    SDL_Point texSize = SDL_TextureSize(texture);
    uint texW = texSize.x;

    double wallLength = raycast.CollisionWall.Length;
    double localX = Vector2Distance(raycast.CollisionWall.a, raycast.CollisionPoint);
    double texCol = (localX / wallLength) * texW;

    texCol *= (wallLength * raycast.CollisionWall.uvScale);
    texCol = fmod(texCol, texW);

    texCol = wrap(texCol, 0, texW - 1);

    SDL_SetTextureColorMod(texture, shadeByte, shadeByte, shadeByte);
    DrawTextureColumn(texture, texCol, col, y, height);

    setColorUint(fogColor);
    draw_rect(col, y, 1, height);

    setColorUint(0xFFFFFFFF);
}

// TODO: Find a way to blend the fog color with the shade color so actors are affected by fog
void RenderActorCol(Level *l, int col, Vector2 position, double rotation) {
    double angle = atan2(col - WindowWidth() / 2, WindowWidth() / 2) + rotation;

    RayCastResult raycast = HitscanLevel(*l, position, angle, false, true, true); // scan actors only

    if (!raycast.Collided) {
        return; // nothing else to do
    }

    double distance = raycast.Distance * cos(angle - l->rotation);

    if (distance == 0) {
        distance = 0.000001;
    }

    if (distance > DepthBuffer[col]) {
        return; // Actor is behind a wall
    }

    double height = WindowHeight() / distance;

    int y = (WindowHeight() - height) / 2;

    y += (GetState()->FakeHeight / distance);


    double shade = fabs(cos((l->rotation + (1.5 * PI)) - raycast.CollisionWall.Angle));
    shade *= (1 - (distance / (WindowWidth() / 2)));
    shade = max(0.6, min(1, shade));
    //shade = floor(shade * 16) / 16;

    byte shadeByte = 255 * shade;

    SDL_Texture *texture = raycast.CollisionWall.tex;
    SDL_Point texSize = SDL_TextureSize(texture);
    uint texW = texSize.x;

    double wallLength = raycast.CollisionWall.Length;
    double localX = Vector2Distance(raycast.CollisionWall.a, raycast.CollisionPoint);
    double texCol = (localX / raycast.CollisionWall.Length) * texW;

    texCol *= (wallLength * raycast.CollisionWall.uvScale);
    texCol = fmod(texCol, texW);

    texCol = wrap(texCol, 0, texW - 1);

    SDL_SetTextureColorMod(texture, shadeByte, shadeByte, shadeByte);
    DrawTextureColumn(texture, texCol, col, y, height);
}

void AddActor(Actor* actor) {
    Level *l = GetState()->level;
    ListAdd(l->actors, actor);
    BakeActorArray(l);
}

void RemoveActor(Actor* actor) {
    Level *l = GetState()->level;
    int idx = ListFind(l->actors, actor);
    if (idx == -1) {
        return;
    }
    ListRemoveAt(l->actors, idx);
    FreeActor(actor);
    BakeActorArray(l);
}

void RenderLevel(Vector2 camPos, double camRot, double fakeHeight) {
    Level *l = GetState()->level;

    byte *sc = getColorUint(l->SkyColor);
    SDL_SetTextureColorMod(skyTex, sc[0], sc[1], sc[2]);
    free(sc);

    double skyPos = remap(camRot, 0, 2 * PI, 0, 256);
    skyPos = (int) skyPos % 256;

    for (int i = -WindowWidth(); i < WindowWidth() * 3; i += 1) {
        double tuSize = 256.0 / WindowWidth();
        double tu = i * tuSize + skyPos;
        SDL_Rect src = {fmod(tu, 256), 0, 1, 256}; // NOLINT(*-narrowing-conversions)
        SDL_Rect dest = {i, 0, 1, WindowHeight() / 2};
        SDL_RenderCopy(GetRenderer(), skyTex, &src, &dest);
    }

    setColorUint(l->FloorColor);
    draw_rect(0, WindowHeight() / 2, WindowWidth(), WindowHeight() / 2);


    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    for (int col = 0; col < WindowWidth(); col++) {
        RenderCol(l, col, camPos, camRot);
        RenderActorCol(l, col, camPos, camRot);
    }
}

void InitSkyTex() {
    skyTex = ToSDLTexture((const unsigned char *) gztex_level_sky, FILTER_LINEAR);
}

