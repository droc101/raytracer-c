//
// Created by droc101 on 4/21/2024.
//

#include "Vector2.h"
#include <math.h>
#include "../defines.h"

Vector2 v2(const float x, const float y) // Create a Vector2
{
	Vector2 v;
	v.x = x;
	v.y = y;
	return v;
}

Vector2 v2s(const float xy) // Create a Vector2 with x and y set the same
{
	Vector2 v;
	v.x = xy;
	v.y = xy;
	return v;
}

Vector2 Vector2FromAngle(const float angle)
{
	return v2(cosf(angle), sinf(angle));
}

float Vector2Angle(const Vector2 a, const Vector2 b)
{
	return acosf(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b)));
}
