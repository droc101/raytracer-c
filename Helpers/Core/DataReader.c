//
// Created by droc101 on 4/27/2024.
//

#include "DataReader.h"

double ReadDouble(const byte *data, int *offset)
{
	double d;
	memcpy(&d, data + *offset, sizeof(double));
	*offset += sizeof(double);
	return d;
}

double ReadDoubleA(const byte *data, const int offset)
{
	double d;
	memcpy(&d, data + offset, sizeof(double));
	return d;
}

uint ReadUint(const byte *data, int *offset)
{
	uint i;
	memcpy(&i, data + *offset, sizeof(uint));
	*offset += sizeof(uint);
	return i;
}

int ReadInt(const byte *data, int *offset)
{
	int i;
	memcpy(&i, data + *offset, sizeof(int));
	*offset += sizeof(int);
	return i;
}

uint ReadUintA(const byte *data, const int offset)
{
	uint i;
	memcpy(&i, data + offset, sizeof(uint));
	return i;
}

float ReadFloat(const byte *data, int *offset)
{
	float f;
	memcpy(&f, data + *offset, sizeof(float));
	*offset += sizeof(float);
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
	memcpy(data + *offset, &d, sizeof(double));
	*offset += sizeof(double);
}

void WriteUint(byte *data, int *offset, uint i)
{
	memcpy(data + *offset, &i, sizeof(uint));
	*offset += sizeof(uint);
}

void WriteInt(byte *data, int *offset, int i)
{
	memcpy(data + *offset, &i, sizeof(int));
	*offset += sizeof(int);
}

void WriteFloat(byte *data, int *offset, float f)
{
	memcpy(data + *offset, &f, sizeof(float));
	*offset += sizeof(float);
}

void WriteByte(byte *data, int *offset, const byte b)
{
	data[*offset] = b;
	*offset += sizeof(byte);
}
