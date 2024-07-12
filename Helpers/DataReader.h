//
// Created by droc101 on 4/27/2024.
//

#ifndef GAME_DATAREADER_H
#define GAME_DATAREADER_H

#include "../defines.h"

// The "A" functions don't increment the offset, they just read from the given offset

/**
 * Reads a double from the given data at the given offset
 * @param data Data to read from
 * @param offset Offset to read from
 * @return The double read
 * @note Increments the offset by 8
 */
double ReadDouble(byte *data, int *offset);

/**
 * Reads a double from the given data at the given offset, but doesn't increment the offset
 * @param data Data to read from
 * @param offset Offset to read from
 * @return The double read
 */
double ReadDoubleA(byte *data, int offset);

/**
 * Reads a uint from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The uint read
 * @note Increments the offset by 4
 */
uint ReadUint(byte *data, int *offset);

/**
 * Reads a uint from the given data at the given offset, but doesn't increment the offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The uint read
 */
uint ReadUintA(byte *data, int offset);

/**
 * Reads a float from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The float read
 * @note Increments the offset by 4
 */
float ReadFloat(byte *data, int *offset);

/**
 * Reads a byte from the given data at the given offset
 * @param data The data to read from
 * @param offset The offset to read from
 * @return The byte read
 * @note Increments the offset by 1
 */
byte ReadByte(byte *data, int *offset);


/**
 * Writes a double to the given data at the given offset
 * @param data The data to write to
 * @param offset The offset to write to
 * @param d The double to write
 * @note Increments the offset by 8
 */
void WriteDouble(byte *data, int *offset, double d);

/**
 * Writes a uint to the given data at the given offset
 * @param data The data to write to
 * @param offset The offset to write to
 * @param i The uint to write
 * @note Increments the offset by 4
 */
void WriteUint(byte *data, int *offset, uint i);

/**
 * Writes a float to the given data at the given offset
 * @param data The data to write to
 * @param offset The offset to write to
 * @param f The float to write
 * @note Increments the offset by 4
 */
void WriteFloat(byte *data, int *offset, float f);

/**
 * Writes a byte to the given data at the given offset
 * @param data The data to write to
 * @param offset The offset to write to
 * @param b The byte to write
 * @note Increments the offset by 1
 */
void WriteByte(byte *data, int *offset, byte b);

#endif //GAME_DATAREADER_H
