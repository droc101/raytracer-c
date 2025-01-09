//
// Created by droc101 on 7/11/2024.
//

#include "Coin.h"
#include <math.h>
#include "../Helpers/Collision.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

void CoinInit(Actor *this)
{
	this->solid = false;
	this->actorWall = CreateWall(v2(0, -0.125),
								 v2(0, 0.125),
								 this->paramB == 1 ? actorTextures[8] : actorTextures[7],
								 1.0,
								 0.0);
	this->paramA = 0;
	this->actorWall->height = 0.25f;
	this->yPosition = -0.25f;
	this->shadowSize = 0.1;
}

void CoinUpdate(Actor *this, double /*delta*/)
{
	if (GetState()->physicsFrame % 8 == 0)
	{
		this->paramA++;
		this->paramA = this->paramA % 4;

		const double uvo = 0.25 * this->paramA;
		this->actorWall->uvOffset = uvo;
	}

	const Vector2 dir = Vector2Sub(GetState()->level->player.pos, this->position);
	this->rotation = atan2(dir.y, dir.x);
	this->rotation += PI;

	if (CollideCylinder(this->position, 0.5, GetState()->level->player.pos))
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
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void CoinDestroy(Actor *this)
{
	FreeWall(this->actorWall);
	free(this->actorWall);
}
