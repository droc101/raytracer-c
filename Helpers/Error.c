//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "../defines.h"
#include "Error.h"
#include "Drawing.h"
#include "Font.h"

_Noreturn void Error(char* error) {
    char buf[256];
    sprintf(buf, "Error: %s\n", error);
    printf("%s\n", buf);
    FontDrawString(vec2(20, 20), buf, 32, 0xFFFF0000);
    SDL_RenderPresent(GetRenderer());
    SDL_Delay(5000);
    exit(1);
}
