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
