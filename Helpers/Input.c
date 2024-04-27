//
// Created by droc101 on 4/20/2024.
//

#include "Input.h"

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
byte keys[SDL_NUM_SCANCODES];

void HandleKeyDown(int code) {
    keys[code] = KS_JUST_PRESSED;
}

void HandleKeyUp(int code) {
    keys[code] = KS_JUST_RELEASED;
}

void UpdateKeyStates() {
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        if (keys[i] == KS_JUST_RELEASED) {
            keys[i] = KS_RELEASED;
        } else if (keys[i] == KS_JUST_PRESSED) {
            keys[i] = KS_PRESSED;
        }
    }
}

bool IsKeyPressed(int code) {
    return keys[code] == KS_PRESSED || keys[code] == KS_JUST_PRESSED;
}

bool IsKeyJustPressed(int code) {
    return keys[code] == KS_JUST_PRESSED;
}

bool IsKeyJustReleased(int code) {
    return keys[code] == KS_JUST_RELEASED;
}
