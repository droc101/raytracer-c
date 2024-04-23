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
    return v;
}

Vector2 vec2s(double xy) // Create a Vector2 with x and y set the same
{
    Vector2 v;
    v.x = xy;
    v.y = xy;
    return v;
}

double Vector2Distance(Vector2 a, Vector2 b) // Get the distance between two Vector2s
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

double Vector2Length(Vector2 vec) {
    return sqrt(vec.x*vec.x+vec.y*vec.y);
}

Vector2 Vector2Normalize(Vector2 vec) {
    double length = Vector2Length(vec);
    return vec2(vec.x / length, vec.y / length);
}

Vector2 Vector2FromAngle(double angle) {
    return vec2(cos(angle), sin(angle));
}

Vector2 Vector2Scale(Vector2 vec, double scale) {
    return vec2(vec.x*scale, vec.y*scale);
}

Vector2 Vector2Add(Vector2 vec, Vector2 offset) {
    return vec2(vec.x+offset.x, vec.y+offset.y);
}

Vector2 Vector2Sub(Vector2 vec, Vector2 offset) {
    return vec2(vec.x-offset.x, vec.y-offset.y);
}

Vector2 Vector2Rotated(Vector2 vec, double angle) {
    Vector2 result = vec2s(2);
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Apply rotation transformation
    result.x = vec.x * cosAngle - vec.y * sinAngle;
    result.y = vec.x * sinAngle + vec.y * cosAngle;

    return result;
}

double Vector2Dot(Vector2 a, Vector2 b) {
    return a.x*b.x + a.y*b.y;
}

double Vector2Angle(Vector2 a, Vector2 b) {
    return acos(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b)));
}

