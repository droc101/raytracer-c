//
// Created by droc101 on 6/22/2024.
//

#include "Collision.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

Vector2 CollideWall(const Wall *w, const Vector2 position, Vector2 moveVec)
{
	const double dx = w->b.x - w->a.x;
	const double dy = w->b.y - w->a.y;
	const int mult = (position.x - w->a.x) * (w->b.y - w->a.y) - (position.y - w->a.y) * (w->b.x - w->a.x) < 0 ? -1 : 1;
	const double hitboxSize = mult * WALL_HITBOX_EXTENTS;
	const Vector2 pos = Vector2Add(position, moveVec);
	const Vector2 hitboxOffset = v2(hitboxSize * dy / w->length, -hitboxSize * dx / w->length);
	if (mult * ((pos.x - w->a.x - hitboxOffset.x) * (w->b.y - w->a.y) -
				(pos.y - w->a.y - hitboxOffset.y) * (w->b.x - w->a.x)) <=
				0 &&
		mult * ((pos.x - w->a.x - hitboxOffset.x) * hitboxOffset.y -
				(pos.y - w->a.y - hitboxOffset.y) * hitboxOffset.x) <=
				0 &&
		mult * ((pos.y - w->b.y - hitboxOffset.y) * hitboxOffset.x -
				(pos.x - w->b.x - hitboxOffset.x) * hitboxOffset.y) <=
				0)
	{
		const double dydx = dy / (dx ? dx : 1);
		const double dxdy = dx / (dy ? dy : 1);
		const double wallLength = w->length;
		moveVec.x = hitboxSize * dy / wallLength +
					(dx == 0   ? w->a.x
					 : dy == 0 ? pos.x
							   : (pos.y - w->a.y + w->a.x * dydx + pos.x * dxdy) / (dydx + dxdy)) -
					position.x;
		moveVec.y = -hitboxSize * dx / wallLength +
					(dx == 0   ? pos.y
					 : dy == 0 ? w->a.y
							   : (pos.x - w->a.x + w->a.y * dxdy + pos.y * dydx) / (dxdy + dydx)) -
					position.y;
	}
	return moveVec;
}

Vector2 Move(Vector2 position, Vector2 moveVec, const void *ignore)
{
	const Level *l = GetState()->level;
	for (int i = 0; i < l->staticWalls->size; i++)
	{
		const Wall *w = SizedArrayGet(l->staticWalls, i);
		if (w == ignore)
		{
			continue;
		}
		moveVec = CollideWall(w, position, moveVec);
	}
	for (int i = 0; i < l->staticActors->size; i++)
	{
		const Actor *a = SizedArrayGet(l->staticActors, i);
		if (a == ignore)
		{
			continue;
		}
		if (a->solid)
		{
			Wall aWall;
			if (GetTransformedWall(a, &aWall))
			{
				moveVec = CollideWall(&aWall, position, moveVec);
			}
		}
	}
	position = Vector2Add(position, moveVec);
	return position;
}

bool CollideCylinder(const Vector2 cylOrigin, const double cylRadius, const Vector2 testPoint)
{
	return Vector2Distance(cylOrigin, testPoint) < cylRadius;
}

bool CollideActorCylinder(const Actor *a, const Vector2 testPoint)
{
	Wall transformedWall;
	if (!GetTransformedWall(a, &transformedWall))
	{
		return false;
	}
	const double radius = transformedWall.length / 2;
	return CollideCylinder(a->position, radius, testPoint);
}
