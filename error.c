//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include "error.h"
#include "Helpers/drawing.h"
#include "Helpers/font.h"
#include "Structs/Vector2.h"

_Noreturn void Error(const char* error) {
    FontDrawString(vec2(20, 20), error, 32);
    SDL_RenderPresent(GetRenderer());
    while (true) {}
}
