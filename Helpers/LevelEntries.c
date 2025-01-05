//
// Created by droc101 on 7/5/2024.
//

#include "LevelEntries.h"
#include "Core/AssetReader.h"
// #include "../Assets/Assets.h"

LevelEntry gLevelEntries[LEVEL_COUNT] = {
	DEFINE_LEVEL("stage1", LEVEL("leveldata_test_level"), "Test Level", true, 1),
	DEFINE_LEVEL("stage2", LEVEL("leveldata_hallway"), "Hallway", true, 2),
	DEFINE_LEVEL("indoor", LEVEL("leveldata_hub"), "Indoor Test", false, -1),
	DEFINE_LEVEL("benchmark", LEVEL("leveldata_benchmark"), "two thousand walls.", true, 3),
	DEFINE_LEVEL("gfjfdjfhdfhgs", LEVEL("leveldata_gfjfdjfhdfhgs"), "gfjfdjfhdfhgs", true, 4),
	STUB_LEVEL(),
	STUB_LEVEL(),
	STUB_LEVEL(),
	STUB_LEVEL(),
	STUB_LEVEL(),
};
