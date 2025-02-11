//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"

#include <box2d/box2d.h>
#include <box2d/types.h>

#include "../Helpers/Core/Error.h"
#include "Vector2.h"
#include "Wall.h"

#include "../Actor/Coin.h"
#include "../Actor/Door.h"
#include "../Actor/Goal.h"
#include "../Actor/TestActor.h"

// Empty template functions
void ActorInit(Actor * /*this*/, b2WorldId /*worldId*/) {}

void ActorUpdate(Actor * /*this*/, double /*delta*/) {}

void ActorDestroy(Actor * /*this*/) {}

ActorInitFunction ActorInitFuncs[] = {
	ActorInit,
	TestActorInit,
	CoinInit,
	GoalInit,
	DoorInit,
};

ActorUpdateFunction ActorUpdateFuncs[] = {
	ActorUpdate,
	TestActorUpdate,
	CoinUpdate,
	GoalUpdate,
	DoorUpdate,
};

ActorDestroyFunction ActorDestroyFuncs[] = {
	ActorDestroy,
	TestActorDestroy,
	CoinDestroy,
	GoalDestroy,
	DoorDestroy,
};

int ActorHealths[] = {
	1,
	1,
	1,
	1,
	1,
};

Actor *CreateActor(const Vector2 position,
				   const float rotation,
				   const int actorType,
				   const byte paramA,
				   const byte paramB,
				   const byte paramC,
				   const byte paramD,
				   const b2WorldId worldId)
{
	Actor *actor = malloc(sizeof(Actor));
	CheckAlloc(actor);
	actor->actorWall = NULL;
	actor->position = position;
	actor->rotation = rotation;
	actor->health = ActorHealths[actorType];
	actor->paramA = paramA;
	actor->paramB = paramB;
	actor->paramC = paramC;
	actor->paramD = paramD;
	actor->yPosition = 0.0f;
	actor->showShadow = true;
	actor->shadowSize = 1.0f;
	actor->actorModel = NULL;
	actor->actorModelTexture = NULL;
	actor->bodyId = b2_nullBodyId;
	ListCreate(&actor->listeningFor);
	actor->SignalHandler = NULL;
	actor->Init = ActorInitFuncs[actorType];
	actor->Update = ActorUpdateFuncs[actorType];
	actor->Destroy = ActorDestroyFuncs[actorType];
	actor->Init(actor, worldId); // kindly allow the Actor to initialize itself
	actor->actorType = actorType;
	return actor;
}

void FreeActor(Actor *actor)
{
	actor->Destroy(actor);
	ListFree(&actor->listeningFor, false);
	free(actor);
	actor = NULL;
}

void ActorListenFor(Actor *actor, const int signal)
{
	ListAdd(&actor->listeningFor, (void *)(size_t)signal);
}

void CreateActorWallCollider(Actor *this, const b2WorldId worldId)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = this->actorWall->a;
	this->bodyId = b2CreateBody(worldId, &bodyDef);
	this->actorWall->bodyId = this->bodyId;
	const float dx = this->actorWall->dx;
	const float dy = this->actorWall->dy;
	const float invDistance = 1 / sqrtf(dx * dx + dy * dy);
	const Vector2 points[4] = {
		{
			(dy - dx / 2) * 0.01f * invDistance,
			(-dx - dy / 2) * 0.01f * invDistance,
		},
		{
			(-dy - dx / 2) * 0.01f * invDistance,
			(dx - dy / 2) * 0.01f * invDistance,
		},
		{
			dx + (dy + dx / 2) * 0.01f * invDistance,
			dy + (-dx + dy / 2) * 0.01f * invDistance,
		},
		{
			dx + (-dy + dx / 2) * 0.01f * invDistance,
			dy + (dx + dy / 2) * 0.01f * invDistance,
		},
	};
	const b2Hull hull = b2ComputeHull(points, 4);
	const b2Polygon shape = b2MakePolygon(&hull, 0);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.friction = 0;
	shapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	b2CreatePolygonShape(this->actorWall->bodyId, &shapeDef, &shape);
}
