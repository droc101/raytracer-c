//
// Created by droc101 on 4/27/2024.
//

#include "DataReader.h"

double ReadDouble(byte *data, int *offset) {
    double d;
    memcpy(&d, data + *offset, sizeof(double));
    *offset += sizeof(double);

    // Reverse the byte order
    for (int i = 0; i < sizeof(double) / 2; i++) {
        byte temp = ((byte *) &d)[i];
        ((byte *) &d)[i] = ((byte *) &d)[sizeof(double) - i - 1];
        ((byte *) &d)[sizeof(double) - i - 1] = temp;
    }

    return d;
}

double ReadDoubleA(byte *data, int offset) {
    double d;
    memcpy(&d, data + offset, sizeof(double));

    // Reverse the byte order
    for (int i = 0; i < sizeof(double) / 2; i++) {
        byte temp = ((byte *) &d)[i];
        ((byte *) &d)[i] = ((byte *) &d)[sizeof(double) - i - 1];
        ((byte *) &d)[sizeof(double) - i - 1] = temp;
    }

    return d;
}

uint ReadUint(byte *data, int *offset) {
    uint i;
    memcpy(&i, data + *offset, sizeof(uint));
    *offset += sizeof(uint);

    // convert to little endian
    i = ((i >> 24) & 0xff) | // move byte 3 to byte 0
        ((i << 8) & 0xff0000) | // move byte 1 to byte 2
        ((i >> 8) & 0xff00) | // move byte 2 to byte 1
        ((i << 24) & 0xff000000); // byte 0 to byte 3

    return i;
}

uint ReadUintA(byte *data, int offset) {
    uint i;
    memcpy(&i, data + offset, sizeof(uint));

    // convert to little endian
    i = ((i >> 24) & 0xff) | // move byte 3 to byte 0
        ((i << 8) & 0xff0000) | // move byte 1 to byte 2
        ((i >> 8) & 0xff00) | // move byte 2 to byte 1
        ((i << 24) & 0xff000000); // byte 0 to byte 3

    return i;
}

float ReadFloat(byte *data, int *offset) {
    float f;
    memcpy(&f, data + *offset, sizeof(float));
    *offset += sizeof(float);

    // Reverse the byte order
    for (int i = 0; i < sizeof(float) / 2; i++) {
        byte temp = ((byte *) &f)[i];
        ((byte *) &f)[i] = ((byte *) &f)[sizeof(float) - i - 1];
        ((byte *) &f)[sizeof(float) - i - 1] = temp;
    }

    return f;
}

byte ReadByte(byte *data, int *offset) {
    byte b = data[*offset];
    *offset += sizeof(byte);
    return b;
}

void WriteDouble(byte *data, int *offset, double d) {
    // Reverse the byte order
    for (int i = 0; i < sizeof(double) / 2; i++) {
        byte temp = ((byte *) &d)[i];
        ((byte *) &d)[i] = ((byte *) &d)[sizeof(double) - i - 1];
        ((byte *) &d)[sizeof(double) - i - 1] = temp;
    }

    memcpy(data + *offset, &d, sizeof(double));
    *offset += sizeof(double);
}

void WriteUint(byte *data, int *offset, uint i) {
    // convert to big endian
    i = ((i >> 24) & 0xff) | // move byte 3 to byte 0
        ((i << 8) & 0xff0000) | // move byte 1 to byte 2
        ((i >> 8) & 0xff00) | // move byte 2 to byte 1
        ((i << 24) & 0xff000000); // byte 0 to byte 3

    memcpy(data + *offset, &i, sizeof(uint));
    *offset += sizeof(uint);
}

void WriteFloat(byte *data, int *offset, float f) {
    // Reverse the byte order
    for (int i = 0; i < sizeof(float) / 2; i++) {
        byte temp = ((byte *) &f)[i];
        ((byte *) &f)[i] = ((byte *) &f)[sizeof(float) - i - 1];
        ((byte *) &f)[sizeof(float) - i - 1] = temp;
    }

    memcpy(data + *offset, &f, sizeof(float));
    *offset += sizeof(float);
}

void WriteByte(byte *data, int *offset, byte b) {
    data[*offset] = b;
    *offset += sizeof(byte);
}
