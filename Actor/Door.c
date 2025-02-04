//
// Created by droc101 on 11/7/2024.
//

#include "Door.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"

typedef enum
{
	DOOR_CLOSED,
	DOOR_OPENING,
	DOOR_OPEN,
	DOOR_CLOSING
} DoorState;

typedef struct DoorData
{
	DoorState state;
	double openingTime;
} DoorData;

void DoorSetState(const Actor *door, const DoorState state)
{
	DoorData *data = door->extra_data;
	data->state = state;
	data->openingTime = 0;
}

void DoorInit(Actor *this, const b2WorldId worldId)
{
	const Vector2 wallOffset = Vector2Normalize((Vector2){cosf(this->rotation), sinf(this->rotation)});
	this->actorWall = malloc(sizeof(Wall));
	CheckAlloc(this->actorWall);
	this->actorWall->a = (Vector2){this->position.x + wallOffset.x, this->position.y + wallOffset.y};
	this->actorWall->b = this->position;
	strncpy(this->actorWall->tex, TEXTURE("actor_door"), 32);
	this->actorWall->uvScale = 1.0f;
	this->actorWall->uvOffset = 0.0f;
	this->actorWall->height = 1.0f;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_kinematicBody;
	bodyDef.position = this->actorWall->a;
	this->bodyId = b2CreateBody(worldId, &bodyDef);
	this->actorWall->bodyId = this->bodyId;
	const b2Segment shape = {
		.point2 = {-wallOffset.x, -wallOffset.y},
	};
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.friction = 0;
	b2CreateSegmentShape(this->bodyId, &shapeDef, &shape);

	this->showShadow = false;
	this->extra_data = malloc(sizeof(DoorData));
	CheckAlloc(this->extra_data);
	memset(this->extra_data, 0, sizeof(DoorData));
	DoorData *data = this->extra_data;
	data->state = DOOR_CLOSED;
	data->openingTime = 0;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void DoorUpdate(Actor *this, const double delta)
{
	this->position = b2Body_GetPosition(this->bodyId);
	DoorData *data = this->extra_data;

	const Vector2 wallCenter = Vector2Scale(Vector2Add(this->actorWall->a, this->actorWall->b), 0.5);
	const bool playerCollide = CollideCylinder(wallCenter, 1.0, GetState()->level->player.pos);

	switch (data->state)
	{
		case DOOR_CLOSED:
			if (playerCollide)
			{
				b2Body_SetLinearVelocity(this->bodyId,
										 Vector2Normalize((Vector2){-cosf(this->rotation), -sinf(this->rotation)}));
				DoorSetState(this, DOOR_OPENING);
			}
			break;
		case DOOR_OPEN:
			if (data->openingTime >= 1 && !playerCollide)
			{
				b2Body_SetLinearVelocity(this->bodyId,
										 Vector2Normalize((Vector2){cosf(this->rotation), sinf(this->rotation)}));
				DoorSetState(this, DOOR_CLOSING);
			}
			break;
		case DOOR_OPENING:
			if (data->openingTime >= 1)
			{
				b2Body_SetLinearVelocity(this->bodyId, (Vector2){0, 0});
				b2Body_SetTransform(this->bodyId, this->actorWall->b, b2MakeRot(0));
				DoorSetState(this, DOOR_OPEN);
			}
			break;
		case DOOR_CLOSING:
			if (data->openingTime >= 1)
			{
				b2Body_SetLinearVelocity(this->bodyId, (Vector2){0, 0});
				b2Body_SetTransform(this->bodyId, this->actorWall->a, b2MakeRot(0));
				DoorSetState(this, DOOR_CLOSED);
			}
			break;
		default:
			LogWarning("Invalid door state: %d", data->state);
			break;
	}
	data->openingTime += delta / PHYSICS_TARGET_TPS;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void DoorDestroy(Actor *this)
{
	free(this->extra_data);
	free(this->actorWall);
}
