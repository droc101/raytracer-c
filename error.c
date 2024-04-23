//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include "defines.h"
#include "error.h"
#include "Helpers/drawing.h"
#include "Helpers/font.h"
#include "Structs/Vector2.h"

_Noreturn void Error(char* error) {
    FontDrawString(vec2(20, 20), error, 32);
    SDL_RenderPresent(GetRenderer());
    SDL_Delay(5000);
    exit(1);
}
