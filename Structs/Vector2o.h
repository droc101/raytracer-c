//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_VECTOR2O_H
#define GAME_VECTOR2O_H

#include "../defines.h"

Vector2o vec2o(double x, double y, double ox, double oy);

Vector2o vec2os(double xy, double ox, double oy);

Vector2 Vector2oToVector2(Vector2o vec);

Vector2o Vector2ToVector2o(Vector2 vec);

#endif //GAME_VECTOR2O_H
