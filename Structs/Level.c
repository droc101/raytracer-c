//
// Created by droc101 on 4/21/2024.
//
#include "Level.h"
#include "../defines.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Vector2.h"
#include "Wall.h"

Level *CreateLevel()
{
	Level *l = malloc(sizeof(Level));
	CheckAlloc(l);
	ListCreate(&l->actors);
	ListCreate(&l->walls);
	ListCreate(&l->triggers);
	l->player.pos = v2s(0);
	l->player.angle = 0;
	l->hasCeiling = false;
	strncpy(l->ceilOrSkyTex, "texture/level_sky_test.gtex", 28);
	strncpy(l->floorTex, "texture/level_floor_test.gtex", 30);
	strncpy(l->music, "none", 5);
	l->fogColor = 0xff000000;
	l->fogStart = 10;
	l->fogEnd = 30;
	strncpy(l->name, "Unnamed Level", 32);
	l->courseNum = -1;
	return l;
}

void DestroyLevel(Level *l)
{
	for (int i = 0; i < l->actors.length; i++)
	{
		Actor *a = ListGet(l->actors, i);
		FreeActor(a);
	}

	ListAndContentsFree(&l->walls, false);
	ListAndContentsFree(&l->triggers, false);
	ListFree(&l->actors, false);
	free(l);
	l = NULL;
}

void AddActor(Actor *actor)
{
	Level *l = GetState()->level;
	ListAdd(&l->actors, actor);
	LoadNewActor();
}

void RemoveActor(Actor *actor)
{
	Level *l = GetState()->level;
	const size_t idx = ListFind(l->actors, actor);
	if (idx == -1)
	{
		return;
	}
	ListRemoveAt(&l->actors, idx);
	FreeActor(actor);
}

void RenderLevel(const GlobalState *g)
{
	RenderLevel3D(g->level, g->cam);
}
