//
// Created by droc101 on 4/20/2024.
//

#include "Input.h"
#include "../../Structs/Vector2.h"

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
byte keys[SDL_NUM_SCANCODES];

byte mouseButtons[4];

int mouseX, mouseY, mouseXrel, mouseYrel;

void HandleMouseMotion(int x, int y, int xrel, int yrel) {
    mouseX = x;
    mouseY = y;
    mouseXrel = xrel;
    mouseYrel = yrel;
}

void HandleMouseDown(int button) {
    mouseButtons[button] = INP_JUST_PRESSED;
}

void HandleMouseUp(int button) {
    mouseButtons[button] = INP_JUST_RELEASED;
}

void HandleKeyDown(int code) {
    keys[code] = INP_JUST_PRESSED;
}

void HandleKeyUp(int code) {
    keys[code] = INP_JUST_RELEASED;
}

void UpdateInputStates() {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        if (keys[i] == INP_JUST_RELEASED) {
            keys[i] = INP_RELEASED;
        } else if (keys[i] == INP_JUST_PRESSED) {
            keys[i] = INP_PRESSED;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (mouseButtons[i] == INP_JUST_RELEASED) {
            mouseButtons[i] = INP_RELEASED;
        } else if (mouseButtons[i] == INP_JUST_PRESSED) {
            mouseButtons[i] = INP_PRESSED;
        }
    }

    mouseXrel = 0;
    mouseYrel = 0;
}

bool IsKeyPressed(int code) {
    return keys[code] == INP_PRESSED || keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustPressed(int code) {
    return keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustReleased(int code) {
    return keys[code] == INP_JUST_RELEASED;
}

bool IsMouseButtonPressed(int button) {
    return mouseButtons[button] == INP_PRESSED || mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustPressed(int button) {
    return mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustReleased(int button) {
    return mouseButtons[button] == INP_JUST_RELEASED;
}

Vector2 GetMousePos() {
    return v2(mouseX, mouseY);
}

Vector2 GetMouseRel() {
    return v2(mouseXrel, mouseYrel);
}

void ConsumeKey(int code) {
    keys[code] = INP_RELEASED;
}

void ConsumeMouseButton(int button) {
    mouseButtons[button] = INP_RELEASED;
}

void ConsumeAllKeys() {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        keys[i] = INP_RELEASED;
    }
}

void ConsumeAllMouseButtons() {
    for (int i = 0; i < 4; i++) {
        mouseButtons[i] = INP_RELEASED;
    }
}
