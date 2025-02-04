//
// Created by droc101 on 4/21/2024.
//

#include "Vector2.h"
#include <math.h>
#include "../defines.h"

inline Vector2 v2(const float x, const float y) // Create a Vector2
{
	return (Vector2){x, y};
}

inline Vector2 v2s(const float xy) // Create a Vector2 with x and y set the same
{
	return (Vector2){xy, xy};
}

inline Vector2 Vector2FromAngle(const float angle)
{
	return v2(cosf(angle), sinf(angle));
}

inline float Vector2Angle(const Vector2 a, const Vector2 b)
{
	return acosf(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b)));
}
