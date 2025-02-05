//
// Created by droc101 on 6/22/2024.
//

#include "Collision.h"
#include <box2d/box2d.h>
#include "../Structs/Actor.h"
#include "../Structs/Vector2.h"

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
