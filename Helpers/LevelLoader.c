//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include "../Helpers/CommonAssets.h"
#include "../Structs/Actor.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"
#include "Core/DataReader.h"
#include "Core/Error.h"
#include "Core/Logging.h"

Level *LoadLevel(const byte *data)
{
	Level *l = CreateLevel();
	// TODO: new level bytecode
	return l;
}
