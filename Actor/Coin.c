//
// Created by droc101 on 7/11/2024.
//

#include "Coin.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/MathEx.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

void CreateCoinSensor(Actor *this, const b2WorldId worldId)
{
	this->extra_data = calloc(1, sizeof(b2ShapeId));
	CheckAlloc(this->extra_data);
	b2ShapeId *shapeId = this->extra_data;

	b2BodyDef sensorBodyDef = b2DefaultBodyDef();
	sensorBodyDef.type = b2_staticBody;
	sensorBodyDef.position = this->position;
	this->bodyId = b2CreateBody(worldId, &sensorBodyDef);
	this->actorWall->bodyId = this->bodyId;
	const b2Circle sensorShape = {
		.radius = 0.5f,
	};
	b2ShapeDef sensorShapeDef = b2DefaultShapeDef();
	sensorShapeDef.isSensor = true;
	*shapeId = b2CreateCircleShape(this->bodyId, &sensorShapeDef, &sensorShape);
}

void CoinInit(Actor *this, const b2WorldId worldId)
{
	this->actorWall = CreateWall((Vector2){this->position.x, this->position.y - 0.125f},
								 (Vector2){this->position.x, this->position.y + 0.125f},
								 this->paramB == 1 ? TEXTURE("actor_bluecoin") : TEXTURE("actor_coin"),
								 1.0f,
								 0.0f);
	WallBake(this->actorWall);

	CreateCoinSensor(this, worldId);

	this->paramA = 0;
	this->actorWall->height = 0.25f;
	this->yPosition = -0.25f;
	this->shadowSize = 0.1f;
}

void CoinUpdate(Actor *this, double /*delta*/)
{
	if (GetState()->physicsFrame % 8 == 0)
	{
		this->paramA++;
		this->paramA %= 4;

		const float uvo = 0.25f * (float)this->paramA;
		this->actorWall->uvOffset = uvo;
	}

	const Vector2 playerPosition = GetState()->level->player.pos;
	const float rotation = atan2f(playerPosition.y - this->position.y, playerPosition.x - this->position.x) + PIf / 2;
	this->actorWall->a = v2(this->position.x - 0.125f * cosf(rotation), this->position.y - 0.125f * sinf(rotation));
	this->actorWall->b = v2(this->position.x + 0.125f * cosf(rotation), this->position.y + 0.125f * sinf(rotation));
	WallBake(this->actorWall);

	const uint32_t sensorShapeIdIndex = ((b2ShapeId *)this->extra_data)->index1;
	const b2SensorEvents sensorEvents = b2World_GetSensorEvents(GetState()->level->worldId);
	for (int i = 0; i < sensorEvents.beginCount; i++)
	{
		const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
		if (event.sensorShapeId.index1 == sensorShapeIdIndex)
		{
			if (this->paramB == 0)
			{
				GetState()->coins++;
			} else
			{
				GetState()->blueCoins++;
				GetState()->coins += 5;
			}
			PlaySoundEffect(SOUND("sfx_coincling"));
			RemoveActor(this);
			break;
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void CoinDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
	free(this->actorWall);
	free(this->extra_data);
}
