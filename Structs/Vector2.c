//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "Vector2.h"
#include <math.h>

Vector2 vec2(double x, double y) // Create a Vector2
{
    Vector2 v;
    v.x = x;
    v.y = y;
    v.originX = 0;
    v.originY = 0;
    return v;
}

Vector2 vec2s(double xy) // Create a Vector2 with x and y set the same
{
    Vector2 v;
    v.x = xy;
    v.y = xy;
    v.originX = 0;
    v.originY = 0;
    return v;
}

Vector2 vec2o(double x, double y, double originX, double originY) // Create a Vector2 with a non-zero origin
{
    Vector2 v;
    v.x = x;
    v.y = y;
    v.originX = originX;
    v.originY = originY;
    return v;
}

Vector2 Vector2RemoveOrigin(Vector2 vec) {
    return vec2(vec.x + vec.originX, vec.y + vec.originY);
}

double Vector2Distance(Vector2 a, Vector2 b) // Get the distance between two Vector2s
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double Vector2Length(Vector2 vec) {
    return sqrt((vec.x - vec.originX) * (vec.x - vec.originX) + (vec.y - vec.originY) * (vec.y - vec.originY));
}

Vector2 Vector2Normalize(Vector2 vec) {
    // Calculate the length of the vector
    double length = Vector2Length(vec);

    // If the length is non-zero, normalize the vector
    if (length != 0) {
        // Calculate the normalized components relative to the origin
        double normalizedX = (vec.x - vec.originX) / length + vec.originX;
        double normalizedY = (vec.y - vec.originY) / length + vec.originY;

        // Return the normalized vector
        return vec2o(normalizedX, normalizedY, vec.originX, vec.originY);
    } else {
        // If the vector has zero length, return the zero vector
        return vec2o(0, 0, vec.originX, vec.originY);
    }
}

Vector2 Vector2FromAngle(double angle) {
    return vec2(cos(angle), sin(angle));
}

Vector2 Vector2Add(Vector2 a, Vector2 b) {
    return vec2o(a.x + b.x - b.originX, a.y + b.y - b.originY, a.originX, a.originY);
}

Vector2 Vector2Sub(Vector2 a, Vector2 b) {
    return vec2o(a.x - b.x + b.originX, a.y - b.y + b.originY, a.originX, a.originY);
}

Vector2 Vector2Scale(Vector2 vec, double scale) {
    return vec2o(vec.x * scale, vec.y * scale, vec.originX, vec.originY);
}


double Vector2Dot(Vector2 a, Vector2 b) {
    return (a.x - a.originX) * (b.x - b.originX) + (a.y - a.originY) * (b.y - b.originY);
}

Vector2 Vector2Rotate(Vector2 vec, double angle) {  // TODO Vec2o
    Vector2 result = vec2s(2);
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Apply rotation transformation
    result.x = vec.x * cosAngle - vec.y * sinAngle;
    result.y = vec.x * sinAngle + vec.y * cosAngle;

    return result;
}

double Vector2Angle(Vector2 a, Vector2 b) {  // TODO Vec2o
    return acos(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b)));
}

