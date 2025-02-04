//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <math.h>
#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/TextBox.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

void GoalInit(Actor *this, b2WorldId /*worldId*/)
{
	this->solid = false;
	this->actorWall = CreateWall(v2(0, -0.5), v2(0, 0.5), TEXTURE("actor_goal0"), 1, 0.0f, GetState()->level->worldId);
}

void GoalUpdate(Actor *this, double /*delta*/)
{
	const Vector2 dir = Vector2Sub(GetState()->level->player.pos, this->position);
	this->rotation = atan2(dir.y, dir.x);
	this->rotation += PI;

	if (CollideActorCylinder(this, GetState()->level->player.pos))
	{
		RemoveActor(this);
		const TextBox tb = DEFINE_TEXT("Goal!",
									   2,
									   20,
									   0,
									   70,
									   TEXT_BOX_H_ALIGN_CENTER,
									   TEXT_BOX_V_ALIGN_TOP,
									   TEXT_BOX_THEME_WHITE);
		ShowTextBox(tb);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GoalDestroy(Actor *this)
{
	free(this->actorWall);
}
