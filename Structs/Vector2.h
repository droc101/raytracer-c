//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_VECTOR2_H
#define GAME_VECTOR2_H

#include "../defines.h"

/**
 * Create a 2D vector with the given x and y values
 * @param x The x value
 * @param y The y value
 * @return The new vector
 */
Vector2 v2(double x, double y);

/**
 * Create a 2D vector with the same x and y values
 * @param xy The x and y value
 * @return The new vector
 */
Vector2 v2s(double xy);

/**
 * Get the distance between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The distance between the two vectors
 */
double Vector2Distance(Vector2 a, Vector2 b);

/**
 * Get the length of a vector
 * @param vec The vector
 * @return The length of the vector
 */
double Vector2Length(Vector2 vec);

/**
 * Normalize a vector
 * @param vec The vector
 * @return The normalized vector
 */
Vector2 Vector2Normalize(Vector2 vec);

/**
 * Create a unit vector from an angle
 * @param angle The angle (in radians)
 * @return The unit vector
 */
Vector2 Vector2FromAngle(double angle);

/**
 * Scale a vector (multiply by a scalar)
 * @param vec The vector
 * @param scale The scalar
 * @return The scaled vector
 */
Vector2 Vector2Scale(Vector2 vec, double scale);

/**
 * Add two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The sum of the two vectors
 */
Vector2 Vector2Add(Vector2 a, Vector2 b);

/**
 * Subtract two vectors
 * @param vec The vector
 * @param offset The offset
 * @return The difference of the two vectors
 */
Vector2 Vector2Sub(Vector2 vec, Vector2 offset);

/**
 * Rotate a vector by an angle
 * @param vec The vector
 * @param angle The angle (in radians)
 * @return The rotated vector
 */
Vector2 Vector2Rotate(Vector2 vec, double angle);

/**
 * Get the dot product of two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The dot product of the two vectors
 */
double Vector2Dot(Vector2 a, Vector2 b);

/**
 * Get the angle between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The angle between the two vectors (in radians)
 */
double Vector2Angle(Vector2 a, Vector2 b);

/**
 * Divide a vector by a divisor
 * @param vec The vector
 * @param divisor The divisor
 * @return The divided vector
 */
Vector2 Vector2Div(Vector2 vec, double divisor);

#endif //GAME_VECTOR2_H
