//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../assets/assets.h"



Wall *CreateWall(Vector2 a, Vector2 b, uint tex);
void FreeWall(Wall w);

// How far out the hitbox of the wall extends from the actual wall (on both sides)
#define WALL_HITBOX_EXTENTS 0.2

double WallGetLength(Wall w);
double WallGetAngle(Wall w);

Vector2 PushPointOutOfWallHitbox(Wall w, Vector2 point);

#endif //GAME_WALL_H
