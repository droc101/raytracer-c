//
// Created by droc101 on 1/18/25.
//

#include "Trigger.h"
#include <box2d/box2d.h>
#include <string.h>
#include "../Helpers/Collision.h"
#include "../Helpers/Core/Error.h"
#include "GlobalState.h"

void CreateTriggerSensor(Trigger *trigger, const Vector2 position, const b2WorldId worldId)
{
	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = position;
	const b2BodyId bodyId = b2CreateBody(worldId, &sensorBodyDef);
	const b2Polygon sensorShape = b2MakeBox(trigger->extents.x * 0.5f, trigger->extents.y * 0.5f);
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	sensorShapeDef.filter.categoryBits = COLLISION_GROUP_TRIGGER;
	sensorShapeDef.filter.maskBits = COLLISION_GROUP_PLAYER;
	trigger->sensorId = b2CreatePolygonShape(bodyId, &sensorShapeDef, &sensorShape);
}

Trigger *CreateTrigger(const Vector2 position,
					   const Vector2 extents,
					   const float rotation,
					   const char *command,
					   const uint flags,
					   const b2WorldId worldId)
{
	Trigger *trigger = malloc(sizeof(Trigger));
	CheckAlloc(trigger);
	trigger->position = position;
	trigger->extents = extents;
	trigger->rotation = rotation;
	trigger->flags = flags;
	strncpy(trigger->command, command, 63);
	trigger->playerColliding = false;

	CreateTriggerSensor(trigger, position, worldId);

	return trigger;
}

bool CheckTriggerCollision(Trigger *trigger)
{
	trigger->playerColliding = GetSensorState(GetState()->level->worldId,
										   trigger->sensorId.index1,
										   trigger->playerColliding);
	return trigger->playerColliding;
}
