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

/* Here's some code for the creation of a wall only actor
 *	this->actorWall = malloc(sizeof(Wall));
 *	CheckAlloc(this->actorWall);
 *	this->actorWall->a = v2(-0.5, 0);
 *	this->actorWall->b = v2(0.5, 0);
 *	strncpy(this->actorWall->tex, TEXTURE("actor_BLOB2"), 32);
 *	this->actorWall->uvScale = 1.0f;
 *	this->actorWall->uvOffset = 0.0f;
 *	this->actorWall->height = 1.0f;
 *
 *	b2BodyDef bodyDef = b2DefaultBodyDef();
 *	bodyDef.type = b2_dynamicBody;
 *	bodyDef.position = this->position;
 *	bodyDef.fixedRotation = true;
 *	bodyDef.linearDamping = 1;
 *	this->actorWall->bodyId = b2CreateBody(worldId, &bodyDef);
 *	const float dx = this->actorWall->b.x - this->actorWall->a.x;
 *	const float dy = this->actorWall->b.y - this->actorWall->a.y;
 *	if (dx != 0 || dy != 0)
 *	{
 *		const float invDistance = 1 / sqrtf(dx * dx + dy * dy);
 *		const Vector2 points[4] = {
 *			{
 *				(dy - dx / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *				(-dx - dy / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *			},
 *			{
 *				(-dy - dx / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *				(dx - dy / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *			},
 *			{
 *				dx + (dy + dx / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *				dy + (-dx + dy / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *			},
 *			{
 *				dx + (-dy + dx / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *				dy + (dx + dy / 2) * WALL_HITBOX_EXTENTS * invDistance,
 *			},
 *		};
 *		const b2Hull hull = b2ComputeHull(points, 4);
 *		const b2Polygon shape = b2MakePolygon(&hull, 0);
 *		const b2ShapeDef shapeDef = b2DefaultShapeDef();
 *		b2CreatePolygonShape(this->actorWall->bodyId, &shapeDef, &shape);
 *	}
 */

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
				   const double rotation,
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

void ActorListenFor(Actor *actor, const int signal)
{
	ListAdd(&actor->listeningFor, (void *)(size_t)signal);
}
