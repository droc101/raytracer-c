//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../assets/assets.h"



Wall CreateWall(Vector2 a, Vector2 b, uint tex);

double WallGetLength(Wall w);
double WallGetAngle(Wall w);

#endif //GAME_WALL_H
