//
// Created by droc101 on 7/4/2024.
//

#ifndef GAME_GLEVELSELECTSTATE_H
#define GAME_GLEVELSELECTSTATE_H

#include "../Assets/Assets.h"
#include "../defines.h"

typedef struct {
    char * displayName;
    unsigned char* levelData;
} LevelEntry;

#define DEFINE_LEVEL(display, data) {display, gzbin_leveldata_##data}

#define STUB_LEVEL() {"", NULLPTR}

#define LEVEL_COUNT 10

void GLevelSelectStateSet();

#endif //GAME_GLEVELSELECTSTATE_H
