//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_VECTOR2_H
#define GAME_VECTOR2_H

#include "../defines.h"

Vector2 vec2(double x, double y);
Vector2 vec2s(double xy);
double Vector2Distance(Vector2 a, Vector2 b);
double Vector2Length(Vector2 vec);
Vector2 Vector2Normalize(Vector2 vec);
Vector2 Vector2FromAngle(double angle);
Vector2 Vector2Scale(Vector2 vec, double scale);
Vector2 Vector2Add(Vector2 vec, Vector2 offset);
Vector2 Vector2Sub(Vector2 vec, Vector2 offset);
Vector2 Vector2Rotated(Vector2 vec, double angle);
double Vector2Dot(Vector2 a, Vector2 b);

#endif //GAME_VECTOR2_H
