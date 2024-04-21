//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_FONT_H
#define GAME_FONT_H

#include "../Structs/Vector2.h"

void FontInit();
void FontDrawString(Vector2 pos, char* str, uint size);

#define FONT_CHAR_WIDTH 16
#define FONT_CHAR_HEIGHT 16
#define FONT_SPACE_WIDTH 8

#endif //GAME_FONT_H
