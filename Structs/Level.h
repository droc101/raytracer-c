//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "../defines.h"
#include "Vector2.h"
#include "Wall.h"
#include "../Helpers/List.h"
#include "SDL.h"

// Create a default level
Level * CreateLevel();

// Free all the memory used by a level
void DestroyLevel(Level *l);

// Render 1 column of the level
void RenderCol(Level *l, int col);

// Render 1 column of the level (actors only, checks depth buffer)
void RenderActorCol(Level *l, int col);

#endif //GAME_LEVEL_H
