//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVELLOADER_H
#define GAME_LEVELLOADER_H

#include "../defines.h"
#include "../Structs/Level.h"

// Level bytecode commands
#define LEVEL_CMD_WALL 0 // Indicates that the next 36 bytes should be parsed as a wall
#define LEVEL_CMD_ACTOR 1 // Load an Actor
#define LEVEL_CMD_PLAYER 2 // Indicates that the next 24 bytes should be parsed as the player's spawn pos and rot
#define LEVEL_CMD_COLORS 3 // Indicates that the next 8 bytes should be parsed as the sky and floor colors
#define LEVEL_CMD_FINISH 4 // Indicates that the level is completed and no further data should be processed
#define LEVEL_CMD_FOG 5 // Indicates that the next 20 bytes should be parsed as the fog color and distances
#define LEVEL_CMD_FLOOR_CEIL 6 // Indicates that the next 8 bytes should be parsed as the floor and ceiling textures
#define LEVEL_CMD_MUSIC 7 // Indicates that the next 4 bytes should be parsed as the music track ID

uint ReadUint(byte *data, int *offset);

uint ReadUintA(byte *data, int offset);

/**
 * Load a level from level bytecode
 * @param data Level bytecode
 * @return Level struct
 */
Level *LoadLevel(byte *data);

typedef struct LevelBytecode
{
    byte *data;
    int size;
} LevelBytecode;

/**
 * Generate level bytecode from a level struct
 * @param l Level struct
 * @return Bytecode struct
 */
LevelBytecode *GenerateBytecode(Level *l);

#endif //GAME_LEVELLOADER_H
