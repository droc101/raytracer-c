//
// Created by droc101 on 7/5/2024.
//

#ifndef GAME_LEVELENTRIES_H
#define GAME_LEVELENTRIES_H

#include "../defines.h"
#include "../Assets/Assets.h"

typedef struct LevelEntry
{
    char *internalName;
    unsigned char *levelData;
    char *displayName;
    bool canPauseExit;
    int courseNum;
} LevelEntry;

#define DEFINE_LEVEL(internal, data, display, canExit, courseNum) {internal, gzbin_leveldata_##data, display, canExit, courseNum}

#define STUB_LEVEL() {" ", NULLPTR, " ", false, 0}

#define LEVEL_COUNT 10

extern LevelEntry gLevelEntries[LEVEL_COUNT];

#endif //GAME_LEVELENTRIES_H
