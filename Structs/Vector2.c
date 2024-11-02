//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "Vector2.h"
#include <math.h>

Vector2 v2(double x, double y) // Create a Vector2
{
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}

Vector2 v2s(double xy) // Create a Vector2 with x and y set the same
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

double Vector2Length(Vector2 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

Vector2 Vector2Normalize(Vector2 vec)
{
    // Calculate the length of the vector
    double length = Vector2Length(vec);

    // If the vector is able to be normalized, continue, otherwise return the input vector
    if (length != 0 && length != 1)
    {
        // Calculate the normalized components and return them as a vector
        return v2(vec.x / length, vec.y / length);
    }
    return vec;
}

Vector2 Vector2FromAngle(double angle)
{
    return v2(cos(angle), sin(angle));
}

Vector2 Vector2Add(Vector2 a, Vector2 b)
{
    return v2(a.x + b.x, a.y + b.y);
}

Vector2 Vector2Sub(Vector2 a, Vector2 b)
{
    return v2(a.x - b.x, a.y - b.y);
}

Vector2 Vector2Scale(Vector2 vec, double scale)
{
    return v2(vec.x * scale, vec.y * scale);
}


double Vector2Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

Vector2 Vector2Rotate(Vector2 vec, double angle)
{
    double cosAngle = cos(angle);
    double sinAngle = sin(angle);

    return v2(vec.x * cosAngle - vec.y * sinAngle, vec.x * sinAngle + vec.y * cosAngle);
}

double Vector2Angle(Vector2 a, Vector2 b)
{
    return acos(Vector2Dot(a, b) / (Vector2Length(a) * Vector2Length(b)));
}

