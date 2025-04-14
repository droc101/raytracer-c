//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include <box2d/box2d.h>
#include <box2d/types.h>

#include "../Helpers/Core/Error.h"
#include "GlobalState.h"
#include "Level.h"

#include "../Actor/Coin.h"
#include "../Actor/Door.h"
#include "../Actor/Goal.h"
#include "../Actor/TestActor.h"
#include "../Helpers/Core/Logging.h"
#include "../Actor/Trigger.h"

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
	TriggerInit
};

ActorUpdateFunction ActorUpdateFuncs[] = {
	ActorUpdate,
	TestActorUpdate,
	CoinUpdate,
	GoalUpdate,
	DoorUpdate,
	TriggerUpdate
};

ActorDestroyFunction ActorDestroyFuncs[] = {
	ActorDestroy,
	TestActorDestroy,
	CoinDestroy,
	GoalDestroy,
	DoorDestroy,
	TriggerDestroy
};

int ActorHealths[] = {
	1,
	1,
	1,
	1,
	1,
	1
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
	ListCreate(&actor->ioConnections);
	actor->SignalHandler = DefaultSignalHandler;
	actor->Init = ActorInitFuncs[actorType];
	actor->Update = ActorUpdateFuncs[actorType];
	actor->Destroy = ActorDestroyFuncs[actorType];
	actor->Init(actor, worldId); // kindly allow the Actor to initialize itself
	actor->actorType = actorType;
	ActorFireOutput(actor, ACTOR_SPAWN_OUTPUT, "");
	return actor;
}

void FreeActor(Actor *actor)
{
	ActorFireOutput(actor, ACTOR_SPAWN_OUTPUT, "");
	actor->Destroy(actor);
	for (int i = 0; i < actor->ioConnections.length; i++)
	{
		ActorConnection *connection = ListGet(actor->ioConnections, i);
		DestroyActorConnection(connection);
	}
	ListFree(&actor->ioConnections, false);
	free(actor);
	actor = NULL;
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

void ActorFireOutput(const Actor *sender, const byte signal, const char *defaultParam)
{
	ListLock(sender->ioConnections);
	for (int i = 0; i < sender->ioConnections.length; i++)
	{
		const ActorConnection *connection = ListGet(sender->ioConnections, i);
		if (connection->myOutput == signal)
		{
			List *actors = GetActorsByName(connection->outActorName, GetState()->level);
			if (actors == NULL)
			{
				LogWarning("Tried to fire signal to actor %s, but it was not found!", connection->outActorName);
				continue;
			}
			for (int j = 0; j < actors->length; j++)
			{
				Actor *actor = ListGet(*actors, j);
				if (actor->SignalHandler != NULL)
				{
					char *param = defaultParam;
					if (connection->outParamOverride[0] != '\0')
					{
						strcpy(param, connection->outParamOverride);
					}
					actor->SignalHandler(actor, sender, connection->targetInput, param);
				}
			}
			ListFree(actors, true);
		}
	}
	ListUnlock(sender->ioConnections);
}

void DestroyActorConnection(ActorConnection *connection)
{
	free(connection);
}

bool DefaultSignalHandler(Actor *self, const Actor *, byte signal, const char *)
{
	if (signal == ACTOR_KILL_INPUT)
	{
		RemoveActor(self);
		return true;
	}
	return false;
}
