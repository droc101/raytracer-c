//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "SDL.h"
#include "../defines.h"

#define KS_RELEASED 0
#define KS_JUST_PRESSED 1
#define KS_PRESSED 2
#define KS_JUST_RELEASED 3

// Event handlers
void HandleKeyDown(int code);
void HandleKeyUp(int code);

// State update
void UpdateKeyStates();

// Exposed methods
// Use SDL_SCANCODE_* codes
bool IsKeyPressed(int code);
bool IsKeyJustPressed(int code);
bool IsKeyJustReleased(int code);

#endif //GAME_INPUT_H
