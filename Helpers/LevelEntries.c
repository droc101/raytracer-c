//
// Created by droc101 on 7/5/2024.
//

#include "LevelEntries.h"
#include "../Assets/Assets.h"

LevelEntry gLevelEntries[LEVEL_COUNT] = { // NOLINT(*-interfaces-global-init)
    DEFINE_LEVEL("stage1", test_level, "Test Level", true, 1),
    DEFINE_LEVEL("stage2", hallway, "Hallway", true, 2),
    DEFINE_LEVEL("indoor", hub, "Indoor Test", false, -1),
    DEFINE_LEVEL("benchmark", benchmark, "two thousand walls.", true, 3),
    DEFINE_LEVEL("gfjfdjfhdfhgs", gfjfdjfhdfhgs, "gfjfdjfhdfhgs", true, 4),
    STUB_LEVEL(),
    STUB_LEVEL(),
    STUB_LEVEL(),
    STUB_LEVEL(),
    STUB_LEVEL()
};
