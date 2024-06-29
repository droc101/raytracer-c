//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include "../defines.h"

// The "A" functions don't increment the offset, they just read from the given offset

double ReadDouble(byte *data, int *offset);

double ReadDoubleA(byte *data, int offset);

uint ReadUint(byte *data, int *offset);

uint ReadUintA(byte *data, int offset);

void WriteDouble(byte *data, int *offset, double d);

void WriteUint(byte *data, int *offset, uint i);

#endif //GAME_DATAREADER_H
