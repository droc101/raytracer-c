//
// Created by droc101 on 4/24/24.
//

#include "Vector2o.h"
#include "Vector2.h"

Vector2o vec2o(double x, double y, double ox, double oy) {
    Vector2o v;
    v.x = x;
    v.y = y;
    v.origin = vec2(ox, oy);
    return v;
}

Vector2o vec2os(double xy, double ox, double oy) {
    return vec2o(xy, xy, ox, oy);
}

Vector2 Vector2oToVector2(Vector2o vec) {
    return vec2(vec.x + vec.origin.x, vec.y + vec.origin.y);
}

Vector2o Vector2ToVector2o(Vector2 vec) {
    return vec2o(vec.x, vec.y, 0, 0);
}

// TODO: Additional functions for Vector2o
