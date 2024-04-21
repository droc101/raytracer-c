//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "../defines.h"
#include "Vector2.h"
#include "wall.h"
#include "../Helpers/List.h"
#include "SDL.h"

Level CreateLevel();
void RenderCol(SDL_Renderer *renderer, Level l, int col);

#endif //GAME_LEVEL_H
