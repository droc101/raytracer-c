//
// Created by droc101 on 4/21/2024.
//

// ReSharper disable CppRedundantInlineSpecifier
#ifndef GAME_VECTOR2_H
#define GAME_VECTOR2_H

#include "../defines.h"

/**
 * Create a 2D vector with the given x and y values
 * @param x The x value
 * @param y The y value
 * @return The new vector
 */
Vector2 v2(float x, float y);

/**
 * Create a 2D vector with the same x and y values
 * @param xy The x and y value
 * @return The new vector
 */
Vector2 v2s(float xy);

/**
 * Create a unit vector from an angle
 * @param angle The angle (in radians)
 * @return The unit vector
 */
Vector2 Vector2FromAngle(float angle);

/**
 * Get the angle between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The angle between the two vectors (in radians)
 */
float Vector2Angle(Vector2 a, Vector2 b);

/**
 * Get the distance between two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The distance between the two vectors
 */
static inline float Vector2Distance(const Vector2 a, const Vector2 b)
{
	return b2Distance(a, b);
}

/**
 * Get the length of a vector
 * @param vec The vector
 * @return The length of the vector
 */
static inline float Vector2Length(const Vector2 vec)
{
	return b2Length(vec);
}

/**
 * Convert a vector into a unit vector if possible
 * @param vec The vector
 * @return The unit vector, or the zero vector if @c vec cannot be turned into a unit vector.
 */
static inline Vector2 Vector2Normalize(const Vector2 vec)
{
	return b2Normalize(vec);
}

/**
 * Add two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The sum of the two vectors
 */
static inline Vector2 Vector2Add(const Vector2 a, const Vector2 b)
{
	return b2Add(a, b);
}

/**
 * Subtract two vectors
 * @param a The vector
 * @param b The offset
 * @return The difference of the two vectors
 */
static inline Vector2 Vector2Sub(const Vector2 a, const Vector2 b)
{
	return b2Sub(a, b);
}

/**
 * Rotate a vector by an angle
 * @param vec The vector
 * @param angle The angle (in radians)
 * @return The rotated vector
 */
static inline Vector2 Vector2Rotate(const Vector2 vec, const float angle)
{
	return b2RotateVector(b2MakeRot(angle), vec);
}

/**
 * Get the dot product of two vectors
 * @param a The first vector
 * @param b The second vector
 * @return The dot product of the two vectors
 */
static inline float Vector2Dot(const Vector2 a, const Vector2 b)
{
	return b2Dot(a, b);
}

/**
 * Scale a vector (multiply by a scalar)
 * @param vec The vector
 * @param scale The scalar
 * @return The scaled vector
 */
static inline Vector2 Vector2Scale(const Vector2 vec, const float scale)
{
	return b2MulSV(scale, vec);
}

/**
 * Divide a vector by a scalar divisor
 * @param vec The vector
 * @param divisor The divisor
 * @return The divided vector
 * @note Prefer to scale by @code 1 / divisor@endcode instead of calling this function
 */
static inline Vector2 Vector2Div(const Vector2 vec, const float divisor)
{
	return b2MulSV(1 / divisor, vec);
}

#endif //GAME_VECTOR2_H
