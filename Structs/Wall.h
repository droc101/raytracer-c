//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../Assets/Assets.h"

int GetTextureCount();

SDL_Texture *LoadWallTexture(int index);

// Create a wall with the given points and texture index
Wall *CreateWall(Vector2 a, Vector2 b, uint tex, float uvScale, float uvOffset);

// Free the memory used by a wall (be warned, this also frees the texture)
void FreeWall(Wall *w);

// How far out the hitbox of the wall extends from the actual wall (on both sides)
#define WALL_HITBOX_EXTENTS 0.4

// Get the length of a wall
//double WallGetLength(Wall w);

// Get the angle of a wall
//double WallGetAngle(Wall w);

double WallBake(Wall *w);

#endif //GAME_WALL_H
