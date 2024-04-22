//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVELLOADER_H
#define GAME_LEVELLOADER_H

#include "../defines.h"
#include "../Structs/level.h"

// Level bytecode commands
#define LEVEL_CMD_WALL 0 // Indicates that the next 36 bytes should be parsed as a wall
#define LEVEL_CMD_ACTOR 1 // @TODO -- DO NOT USE YET
#define LEVEL_CMD_PLAYER 2 // Indicates that the next 24 bytes should be parsed as the player's spawn pos and rot
#define LEVEL_CMD_COLORS 3 // Indicates that the next 8 bytes should be parsed as the sky and floor colors
#define LEVEL_CMD_FINISH 4 // Indicates that the level is completed and no further data should be processed

// Load a level from bytecode
Level *LoadLevel(byte *data);

#endif //GAME_LEVELLOADER_H
