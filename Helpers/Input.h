//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "SDL.h"
#include "../defines.h"

#define INP_RELEASED 0
#define INP_JUST_PRESSED 1
#define INP_PRESSED 2
#define INP_JUST_RELEASED 3

// Event handlers
void HandleKeyDown(int code);
void HandleKeyUp(int code);
void HandleMouseMotion(int x, int y, int xrel, int yrel);
void HandleMouseDown(int button);
void HandleMouseUp(int button);

// State update
void UpdateInputStates();

// Exposed methods
// Use SDL_SCANCODE_* codes
bool IsKeyPressed(int code);
bool IsKeyJustPressed(int code);
bool IsKeyJustReleased(int code);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonJustPressed(int button);
bool IsMouseButtonJustReleased(int button);
Vector2 GetMousePos();
Vector2 GetMouseRel();

#endif //GAME_INPUT_H
