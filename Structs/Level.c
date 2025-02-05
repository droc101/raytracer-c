//
// Created by droc101 on 4/21/2024.
//

#include "Level.h"
#include <box2d/box2d.h>
#include <string.h>
#include "../defines.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Graphics/RenderingHelpers.h"
#include "Actor.h"
#include "GlobalState.h"
#include "Vector2.h"

Level *CreateLevel()
{
	Level *l = malloc(sizeof(Level));
	CheckAlloc(l);
	ListCreate(&l->actors);
	ListCreate(&l->walls);
	ListCreate(&l->triggers);
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity.y = 0;
	l->worldId = b2CreateWorld(&worldDef);
	l->player.pos = v2s(0);
	l->player.angle = 0;
	b2BodyDef playerBodyDef = b2DefaultBodyDef();
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.position = l->player.pos;
	playerBodyDef.fixedRotation = true;
	playerBodyDef.linearDamping = 12;
	l->player.bodyId = b2CreateBody(l->worldId, &playerBodyDef);
	const b2Circle playerShape = {
		.center = l->player.pos,
		.radius = 0.25f,
	};
	const b2ShapeDef playerShapeDef = b2DefaultShapeDef();
	b2CreateCircleShape(l->player.bodyId, &playerShapeDef, &playerShape);
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
	b2DestroyWorld(l->worldId);

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
