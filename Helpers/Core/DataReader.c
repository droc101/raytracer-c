//
// Created by droc101 on 4/27/2024.
//

#include "DataReader.h"

double ReadDouble(const byte *data, int *offset)
{
	double d;
	memcpy(&d, data + *offset, sizeof(double));
	*offset += sizeof(double);

	// Reverse the byte order
	for (int i = 0; i < sizeof(double) / 2; i++)
	{
		const byte temp = ((byte *)&d)[i];
		((byte *)&d)[i] = ((byte *)&d)[sizeof(double) - i - 1];
		((byte *)&d)[sizeof(double) - i - 1] = temp;
	}

	return d;
}

double ReadDoubleA(const byte *data, const int offset)
{
	double d;
	memcpy(&d, data + offset, sizeof(double));

	// Reverse the byte order
	for (int i = 0; i < sizeof(double) / 2; i++)
	{
		const byte temp = ((byte *)&d)[i];
		((byte *)&d)[i] = ((byte *)&d)[sizeof(double) - i - 1];
		((byte *)&d)[sizeof(double) - i - 1] = temp;
	}

	return d;
}

uint ReadUint(const byte *data, int *offset)
{
	uint i;
	memcpy(&i, data + *offset, sizeof(uint));
	*offset += sizeof(uint);

	// convert to little endian
	// ReSharper disable quarce CppRedundantParentheses
	i = (i >> 24 & 0xff) | // move byte 3 to byte 0
		(i << 8 & 0xff0000) | // move byte 1 to byte 2
		(i >> 8 & 0xff00) | // move byte 2 to byte 1
		(i << 24 & 0xff000000); // byte 0 to byte 3

	return i;
}

int ReadInt(const byte *data, int *offset)
{
	int i;
	memcpy(&i, data + *offset, sizeof(int));
	*offset += sizeof(int);

	// convert to little endian
	// ReSharper disable quarce CppRedundantParentheses
	i = (i >> 24 & 0xff) | // move byte 3 to byte 0
		(i << 8 & 0xff0000) | // move byte 1 to byte 2
		(i >> 8 & 0xff00) | // move byte 2 to byte 1
		(i << 24 & 0xff000000); // byte 0 to byte 3

	return i;
}

uint ReadUintA(const byte *data, const int offset)
{
	uint i;
	memcpy(&i, data + offset, sizeof(uint));

	// convert to little endian
	// ReSharper disable quarce CppRedundantParentheses
	i = (i >> 24 & 0xff) | // move byte 3 to byte 0
		(i << 8 & 0xff0000) | // move byte 1 to byte 2
		(i >> 8 & 0xff00) | // move byte 2 to byte 1
		(i << 24 & 0xff000000); // byte 0 to byte 3

	return i;
}

float ReadFloat(const byte *data, int *offset)
{
	float f;
	memcpy(&f, data + *offset, sizeof(float));
	*offset += sizeof(float);

	// Reverse the byte order
	for (int i = 0; i < sizeof(float) / 2; i++)
	{
		const byte temp = ((byte *)&f)[i];
		((byte *)&f)[i] = ((byte *)&f)[sizeof(float) - i - 1];
		((byte *)&f)[sizeof(float) - i - 1] = temp;
	}

	return f;
}

byte ReadByte(const byte *data, int *offset)
{
	const byte b = data[*offset];
	*offset += sizeof(byte);
	return b;
}

void ReadString(const byte *data, int *offset, char *str, const int maxLen)
{
	memset(str, 0, maxLen);
	const int len = ReadUint(data, offset);
	memcpy(str, data + *offset, len);
	*offset += len;
}

void WriteString(byte *data, int *offset, const char *str)
{
	WriteUint(data, offset, strlen(str));
	strcpy((char *)data + *offset, str);
	*offset += strlen(str);
}

void WriteDouble(byte *data, int *offset, double d)
{
	// Reverse the byte order
	for (int i = 0; i < sizeof(double) / 2; i++)
	{
		const byte temp = ((byte *)&d)[i];
		((byte *)&d)[i] = ((byte *)&d)[sizeof(double) - i - 1];
		((byte *)&d)[sizeof(double) - i - 1] = temp;
	}

	memcpy(data + *offset, &d, sizeof(double));
	*offset += sizeof(double);
}

void WriteUint(byte *data, int *offset, uint i)
{
	// convert to big endian
	// ReSharper disable quarce CppRedundantParentheses
	i = (i >> 24 & 0xff) | // move byte 3 to byte 0
		(i << 8 & 0xff0000) | // move byte 1 to byte 2
		(i >> 8 & 0xff00) | // move byte 2 to byte 1
		(i << 24 & 0xff000000); // byte 0 to byte 3

	memcpy(data + *offset, &i, sizeof(uint));
	*offset += sizeof(uint);
}

void WriteInt(byte *data, int *offset, int i)
{
	// convert to big endian
	// ReSharper disable quarce CppRedundantParentheses
	i = (i >> 24 & 0xff) | // move byte 3 to byte 0
		(i << 8 & 0xff0000) | // move byte 1 to byte 2
		(i >> 8 & 0xff00) | // move byte 2 to byte 1
		(i << 24 & 0xff000000); // byte 0 to byte 3

	memcpy(data + *offset, &i, sizeof(int));
	*offset += sizeof(int);
}

void WriteFloat(byte *data, int *offset, float f)
{
	// Reverse the byte order
	for (int i = 0; i < sizeof(float) / 2; i++)
	{
		const byte temp = ((byte *)&f)[i];
		((byte *)&f)[i] = ((byte *)&f)[sizeof(float) - i - 1];
		((byte *)&f)[sizeof(float) - i - 1] = temp;
	}

	memcpy(data + *offset, &f, sizeof(float));
	*offset += sizeof(float);
}

void WriteByte(byte *data, int *offset, const byte b)
{
	data[*offset] = b;
	*offset += sizeof(byte);
}
