//
// Created by droc101 on 11/7/2024.
//

#include "Door.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

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
	bool playerColliding;
	double animationTime;
	b2ShapeId sensorId;
} DoorData;

void DoorSetState(const Actor *door, const DoorState state)
{
	DoorData *data = door->extra_data;
	data->state = state;
	data->animationTime = 0;
}

void DoorCreateCollider(Actor *this, const b2WorldId worldId, const Vector2 wallOffset)
{
	b2BodyDef doorBodyDef = b2DefaultBodyDef();
	doorBodyDef.type = b2_kinematicBody;
	doorBodyDef.position = this->actorWall->a;
	this->bodyId = b2CreateBody(worldId, &doorBodyDef);
	this->actorWall->bodyId = this->bodyId;
	const b2Segment doorShape = {
		.point1 = {-wallOffset.x, -wallOffset.y},
		.point2 = {wallOffset.x, wallOffset.y},
	};
	b2ShapeDef doorShapeDef = b2DefaultShapeDef();
	doorShapeDef.friction = 0;
	doorShapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	b2CreateSegmentShape(this->bodyId, &doorShapeDef, &doorShape);
}

void DoorCreateSensor(Actor *this, const b2WorldId worldId)
{
	this->extra_data = calloc(1, sizeof(DoorData));
	CheckAlloc(this->extra_data);
	DoorData *data = this->extra_data;

	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = Vector2Scale(Vector2Add(this->actorWall->a, this->actorWall->b), 0.5f);
	const b2BodyId sensorBody = b2CreateBody(worldId, &sensorBodyDef);
	const b2Circle sensorShape = {
		.radius = 1,
	};
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	sensorShapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	sensorShapeDef.filter.maskBits = COLLISION_GROUP_PLAYER;
	data->sensorId = b2CreateCircleShape(sensorBody, &sensorShapeDef, &sensorShape);
}

void DoorInit(Actor *this, const b2WorldId worldId)
{
	const Vector2 wallOffset = Vector2Scale(Vector2Normalize((Vector2){-cosf(this->rotation), -sinf(this->rotation)}),
											0.5f);
	this->actorWall = CreateWall((Vector2){this->position.x - wallOffset.x, this->position.y - wallOffset.y},
								 (Vector2){this->position.x + wallOffset.x, this->position.y + wallOffset.y},
								 TEXTURE("actor_door"),
								 1.0f,
								 0.0f);
	WallBake(this->actorWall);

	DoorCreateCollider(this, worldId, wallOffset);
	DoorCreateSensor(this, worldId);

	DoorData *data = this->extra_data; // Allocated in CreateSensor
	this->showShadow = false;
	data->state = DOOR_CLOSED;
	data->animationTime = 0;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void DoorUpdate(Actor *this, const double delta)
{
	this->position = b2Body_GetPosition(this->bodyId);
	DoorData *data = this->extra_data;

	const uint32_t sensorShapeIdIndex = data->sensorId.index1;
	const b2SensorEvents sensorEvents = b2World_GetSensorEvents(GetState()->level->worldId);
	if (data->playerColliding)
	{
		for (int i = 0; i < sensorEvents.endCount; i++)
		{
			const b2SensorEndTouchEvent event = sensorEvents.endEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				data->playerColliding = false;
				break;
			}
		}
	} else
	{
		for (int i = 0; i < sensorEvents.beginCount; i++)
		{
			const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				data->playerColliding = true;
				break;
			}
		}
	}

	switch (data->state)
	{
		case DOOR_CLOSED:
			if (data->playerColliding)
			{
				b2Body_SetLinearVelocity(this->bodyId,
										 Vector2Normalize((Vector2){-cosf(this->rotation), -sinf(this->rotation)}));
				DoorSetState(this, DOOR_OPENING);
			}
			break;
		case DOOR_OPEN:
			if (data->animationTime >= 1 && !data->playerColliding)
			{
				b2Body_SetLinearVelocity(this->bodyId,
										 Vector2Normalize((Vector2){cosf(this->rotation), sinf(this->rotation)}));
				DoorSetState(this, DOOR_CLOSING);
			}
			break;
		case DOOR_OPENING:
			if (data->animationTime >= 1)
			{
				b2Body_SetLinearVelocity(this->bodyId, (Vector2){0, 0});
				b2Body_SetTransform(this->bodyId, this->actorWall->b, b2MakeRot(0));
				DoorSetState(this, DOOR_OPEN);
			}
			break;
		case DOOR_CLOSING:
			if (data->playerColliding)
			{
				b2Body_SetLinearVelocity(this->bodyId,
										 Vector2Normalize((Vector2){-cosf(this->rotation), -sinf(this->rotation)}));
				data->state = DOOR_OPENING; // Set manually in order to not reset data->animationTime
				data->animationTime = 1 - data->animationTime;
			} else if (data->animationTime >= 1)
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
	data->animationTime += delta / PHYSICS_TARGET_TPS;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void DoorDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	b2DestroyBody(b2Shape_GetBody(((DoorData *)this->extra_data)->sensorId));
	free(this->extra_data);
	free(this->actorWall);
}
