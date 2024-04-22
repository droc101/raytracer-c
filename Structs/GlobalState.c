//
// Created by droc101 on 4/22/2024.
//

#include "GlobalState.h"
#include "../Structs/level.h"

GlobalState state;

void InitState() {
    state.hp = 100;
    state.maxHp = 100;
    state.ammo = 100;
    state.maxAmmo = 100;
    state.frame = 0;
    state.level = CreateLevel(); // empty level so we don't segfault
    state.requestExit = false;
}

GlobalState *GetState() {
    return &state;
}

void TakeDamage(int damage) {
    state.hp -= damage;
    if (state.hp < 0) {
        state.hp = 0;
    }
}

void Heal(int amount) {
    state.hp += amount;
    if (state.hp > state.maxHp) {
        state.hp = state.maxHp;
    }
}

void AddAmmo(int amount) {
    state.ammo += amount;
    if (state.ammo > state.maxAmmo) {
        state.ammo = state.maxAmmo;
    }
}

void UseAmmo(int amount) {
    state.ammo -= amount;
    if (state.ammo < 0) {
        state.ammo = 0;
    }
}

void SetUpdateCallback(void (*UpdateGame)()) {
    state.frame = 0;
    state.UpdateGame = UpdateGame;
}

void SetRenderCallback(void (*RenderGame)()) {
    state.RenderGame = RenderGame;
}

void ChangeLevel(Level *l) {
    DestroyLevel(state.level);
    state.level = l;
}
