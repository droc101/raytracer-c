//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_VECTOR2_H
#define GAME_VECTOR2_H

#include "../defines.h"

// Create a 2D vector with the given x and y values
Vector2 v2(double x, double y);

// Create a 2D vector with the same x and y values
Vector2 v2s(double xy);

// Get the distance between two vectors
double Vector2Distance(Vector2 a, Vector2 b);

// Get the length of a vector
double Vector2Length(Vector2 vec);

// Normalize a vector
Vector2 Vector2Normalize(Vector2 vec);

// Get the angle of a vector
Vector2 Vector2FromAngle(double angle);

// Scale a vector (multiply by a scalar)
Vector2 Vector2Scale(Vector2 vec, double scale);

// Add two vectors
Vector2 Vector2Add(Vector2 a, Vector2 b);

// Subtract two vectors
Vector2 Vector2Sub(Vector2 vec, Vector2 offset);

// Rotate a vector by an angle
Vector2 Vector2Rotate(Vector2 vec, double angle);

// Get the dot product of two vectors
double Vector2Dot(Vector2 a, Vector2 b);

// Get the angle between two vectors
double Vector2Angle(Vector2 a, Vector2 b);

#endif //GAME_VECTOR2_H
