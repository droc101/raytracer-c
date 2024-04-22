//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_GLOBALSTATE_H
#define GAME_GLOBALSTATE_H

#include "../defines.h"

void InitState();

GlobalState *GetState();

void TakeDamage(int damage);

void Heal(int amount);

void AddAmmo(int amount);

void UseAmmo(int amount);

void SetUpdateCallback(void (*UpdateGame)());
void SetRenderCallback(void (*RenderGame)());

void ChangeLevel(Level *l);

#endif //GAME_GLOBALSTATE_H
