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
	chk_malloc(l);
	l->actors = CreateList();
	l->walls = CreateList();
	l->player.pos = v2s(0);
	l->player.angle = 0;
	l->skyColor = 0xff82c5ff;
	l->floorTexture = 0;
	l->ceilingTexture = 0;
	l->musicID = 0;
	l->fogColor = 0xff000000;
	l->fogStart = 10;
	l->fogEnd = 30;
	strncpy(l->name, "Unnamed Level", 32);
	l->courseNum = -1;
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

	ListFreeWithData(l->walls);
	ListFree(l->actors); // actors are freed above (FreeActor)
	free(l);
	l = NULL;
}

void AddActor(Actor *actor)
{
	const Level *l = GetState()->level;
	ListAdd(l->actors, actor);
}

void RemoveActor(Actor *actor)
{
	const Level *l = GetState()->level;
	const int idx = ListFind(l->actors, actor);
	if (idx == -1)
	{
		return;
	}
	ListRemoveAt(l->actors, idx);
	FreeActor(actor);
}

void RenderLevel(const GlobalState *g)
{
	RenderLevel3D(g->level, g->cam);
}
