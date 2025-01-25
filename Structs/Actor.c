//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include "../Helpers/Core/Error.h"
#include "Vector2.h"
#include "Wall.h"

#include "../Actor/Coin.h"
#include "../Actor/Door.h"
#include "../Actor/Goal.h"
#include "../Actor/TestActor.h"

// Empty template functions
void ActorInit(Actor * /*this*/) {}

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
				   const double rotation,
				   const int actorType,
				   const byte paramA,
				   const byte paramB,
				   const byte paramC,
				   const byte paramD)
{
	Actor *actor = malloc(sizeof(Actor));
	chk_malloc(actor);
	actor->actorWall = NULL;
	actor->position = position;
	actor->rotation = rotation;
	actor->solid = false;
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
	actor->listeningFor = CreateList();
	actor->SignalHandler = NULL;
	actor->Init = ActorInitFuncs[actorType];
	actor->Update = ActorUpdateFuncs[actorType];
	actor->Destroy = ActorDestroyFuncs[actorType];
	actor->Init(actor); // kindly allow the Actor to initialize itself
	actor->actorType = actorType;
	return actor;
}

void FreeActor(Actor *actor)
{
	actor->Destroy(actor);
	ListFree(actor->listeningFor);
	free(actor);
	actor = NULL;
}

bool GetTransformedWall(const Actor *actor, Wall *wall)
{
	if (actor->actorWall == NULL)
	{
		return false;
	}

	memcpy(wall, actor->actorWall, sizeof(Wall)); // duplicate the wall struct without modifying the original

	// Rotate the wall
	wall->a = Vector2Rotate(wall->a, actor->rotation);
	wall->b = Vector2Rotate(wall->b, actor->rotation);
	// Translate the wall
	wall->a = Vector2Add(wall->a, actor->position);
	wall->b = Vector2Add(wall->b, actor->position);

	WallBake(wall);

	return true;
}

void ActorListenFor(const Actor *actor, const int signal)
{
	ListAdd(actor->listeningFor, (void *)((size_t)signal));
}
