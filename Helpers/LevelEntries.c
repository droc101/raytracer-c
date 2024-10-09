//
// Created by droc101 on 7/5/2024.
//

#include "LevelEntries.h"

LevelEntry gLevelEntries[LEVEL_COUNT] = {
        DEFINE_LEVEL("stage1", test_level, "Test Level", true, 1),
        DEFINE_LEVEL("stage2", hallway, "Hallway", true, 2),
        DEFINE_LEVEL("indoor", hub, "Indoor Test", false, -1),
        DEFINE_LEVEL("benchmark", benchmark, "two thousand walls.", true, 3),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL(),
        STUB_LEVEL()
};
