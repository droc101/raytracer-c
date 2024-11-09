//
// Created by droc101 on 4/21/2024.
//
#include "Level.h"
#include <math.h>
#include "Actor.h"
#include "GlobalState.h"
#include "Vector2.h"
#include "Wall.h"
#include "../defines.h"
#include "../Assets/Assets.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/RenderingHelpers.h"

Level *CreateLevel()
{
    Level *l = malloc(sizeof(Level));
    l->actors = CreateList();
    l->walls = CreateList();
    l->position = v2s(0);
    l->rotation = 0;
    l->SkyColor = 0xff82c5ff;
    l->FloorTexture = 0;
    l->CeilingTexture = 0;
    l->MusicID = 0;
    l->FogColor = 0xff000000;
    l->FogStart = 10;
    l->FogEnd = 30;
    l->staticWalls = NULLPTR;
    l->staticActors = NULLPTR;
    return l;
}

void DestroyLevel(Level *l)
{
    for (int i = 0; i < l->walls->size; i++)
    {
        Wall *w = ListGet(l->walls, i);
        FreeWall(w);
    }
    for (int i = 0; i < l->actors->size; i++)
    {
        Actor *a = ListGet(l->actors, i);
        FreeActor(a);
    }

    if (l->staticWalls != NULL)
    {
        DestroySizedArray(l->staticWalls);
    }

    if (l->staticActors != NULL)
    {
        DestroySizedArray(l->staticActors);
    }

    ListFreeWithData(l->walls);
    ListFree(l->actors); // actors are freed above (FreeActor)
    free(l);
}

void BakeWallArray(Level *l)
{
    l->staticWalls = ToSizedArray(l->walls);
}

void BakeActorArray(Level *l)
{
    l->staticActors = ToSizedArray(l->actors);
}

void AddActor(Actor *actor)
{
    Level *l = GetState()->level;
    ListAdd(l->actors, actor);
    BakeActorArray(l);
}

void RemoveActor(Actor *actor)
{
    Level *l = GetState()->level;
    const int idx = ListFind(l->actors, actor);
    if (idx == -1)
    {
        return;
    }
    ListRemoveAt(l->actors, idx);
    FreeActor(actor);
    BakeActorArray(l);
}

void RenderLevelSky(const Camera *cam)
{
    const Level *l = GetState()->level;

    const double camRot = cam->yaw;

    const Vector2 wndSize = v2(WindowWidth(), WindowHeight());

    const int skyPos = (int) (camRot * 128 / PI) % 256;
    const int height = wndSize.y / 2;
    const int offset = (int) (wndSize.x * (1 - skyPos / 256.0));

    DrawTextureRegionMod(v2(offset, 0), v2(wndSize.x * skyPos / 256, height), gztex_level_sky, v2(0, 0),
                         v2(skyPos, 128), l->SkyColor);
    DrawTextureRegionMod(v2(0, 0), v2(offset, height), gztex_level_sky, v2(skyPos, 0), v2(256 - skyPos, 128),
                         l->SkyColor);
}

void RenderLevel(const GlobalState *g)
{
    RenderLevelSky(g->cam);
    RenderLevel3D(g->level, g->cam);
}
