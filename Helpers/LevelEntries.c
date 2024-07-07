//
// Created by droc101 on 7/5/2024.
//

#include "LevelEntries.h"

LevelEntry gLevelEntries[LEVEL_COUNT] = {
        DEFINE_LEVEL("stage1", test_level, "Test Level", true),
        DEFINE_LEVEL("stage2", hallway, "Hallway", true),
        DEFINE_LEVEL("indoor", hub, "Indoor Test", false),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL()
};
