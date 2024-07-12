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
 * @param xrel Relative X position
 * @param yrel Relative Y position
 */
void HandleMouseMotion(int x, int y, int xrel, int yrel);
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
 * @note Only call this once per frame
 */
void UpdateInputStates();

// Exposed methods
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

#endif //GAME_INPUT_H
