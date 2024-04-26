//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_GLOBALSTATE_H
#define GAME_GLOBALSTATE_H

#include "../defines.h"

// Please only call this function once
void InitState();

// Get the global state
GlobalState *GetState();

void TakeDamage(int damage);

void Heal(int amount);

void AddAmmo(int amount);

void UseAmmo(int amount);

// Set the update and render functions
void SetUpdateCallback(void (*UpdateGame)());
void SetRenderCallback(void (*RenderGame)());

// Set the level
void ChangeLevel(Level *l);

#endif //GAME_GLOBALSTATE_H
