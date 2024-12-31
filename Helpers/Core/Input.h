//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "../../defines.h"

#define INP_RELEASED 0
#define INP_JUST_PRESSED 1
#define INP_PRESSED 2
#define INP_JUST_RELEASED 3

void HandleControllerDisconnect(Sint32 which);

void HandleControllerConnect();

void HandleControllerButtonUp(SDL_GameControllerButton button);

void HandleControllerButtonDown(SDL_GameControllerButton button);

void HandleControllerAxis(SDL_GameControllerAxis axis, Sint16 value);

/**
 * Handles key down event
 * @param code Key code
 */
void HandleKeyDown(int code);

/**
 * Handles key up event
 * @param code Key code
 */
void HandleKeyUp(int code);

/**
 * Handles mouse motion event
 * @param x X position
 * @param y Y position
 * @param xRel Relative X position
 * @param yRel Relative Y position
 */
void HandleMouseMotion(int x, int y, int xRel, int yRel);

/**
 * Handles mouse down event
 * @param button Button code
 */
void HandleMouseDown(int button);

/**
 * Handles mouse up event
 * @param button Button code
 */
void HandleMouseUp(int button);

/**
 * Updates input states
 * @note Only call this once per physicsFrame
 */
void UpdateInputStates();

// Exposed methods

/**
 * Checks if a controller button is pressed
 * @param button The button code
 * @return Whether the button is pressed
 */
bool IsButtonPressed(int button);

/**
 * Checks if a controller button is just pressed
 * @param button The button code
 * @return Whether the button is just pressed
 */
bool IsButtonJustPressed(int button);

/**
 * Checks if a controller button is just released
 * @param button The button code
 * @return Whether the button is just released
 */
bool IsButtonJustReleased(int button);

/**
 * Checks if a key is pressed
 * @param code Key code
 * @return Whether the key is pressed
 */
bool IsKeyPressed(int code);

/**
 * Checks if a key is just pressed
 * @param code Key code
 * @return Whether the key is just pressed
 */
bool IsKeyJustPressed(int code);

/**
 * Checks if a key is just released
 * @param code Key code
 * @return Whether the key is just released
 */
bool IsKeyJustReleased(int code);

/**
 * Checks if a mouse button is pressed
 * @param button Button code
 * @return Whether the button is pressed
 */
bool IsMouseButtonPressed(int button);

/**
 * Checks if a mouse button is just pressed
 * @param button Button code
 * @return Whether the button is just pressed
 */
bool IsMouseButtonJustPressed(int button);

/**
 * Checks if a mouse button is just released
 * @param button Button code
 * @return Whether the button is just released
 */
bool IsMouseButtonJustReleased(int button);

/**
 * Gets the mouse position
 * @return The current mouse position
 */
Vector2 GetMousePos();

/**
 * Gets the relative mouse movement
 * @return relative mouse movement
 */
Vector2 GetMouseRel();

/**
 * Consumes a key press state, so no other input check can see it
 * @param code The key code
 */
void ConsumeKey(int code);

/**
 * Consumes a controller button press state, so no other input check can see it
 * @param btn The button code
 */
void ConsumeButton(int btn);

/**
 * Consumes a mouse button press state, so no other input check can see it
 * @param button The button code
 */
void ConsumeMouseButton(int button);

/**
 * Consumes all key press states, so no other input check can see them
 */
void ConsumeAllKeys();

/**
 * Consumes all mouse button press states, so no other input check can see them
 */
void ConsumeAllMouseButtons();

/**
 * Gets the value of a controller axis
 * @param axis The axis to get the value of
 * @return The value of the axis (between -1 and 1)
 */
double GetAxis(SDL_GameControllerAxis axis);

/**
 * Checks if a controller is being used
 * @return whether a controller is being used
 */
bool UseController();

/**
 * Rumble the controller (if available)
 * @param strength The base strength of the rumble (0.0 - 1.0)
 * @param time The time to rumble in milliseconds
 */
void Rumble(const float strength, const uint time);

#endif //GAME_INPUT_H
