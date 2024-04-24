//
// Created by droc101 on 4/21/2024.
//

#include <math.h>

double min(double a, double b) // Get the minimum of two numbers
{
    return a < b ? a : b;
}

double max(double a, double b) // Get the maximum of two numbers
{
    return a > b ? a : b;
}

double wrap(double n, double min, double max) {
    double d = max - min;
    return n >= min && n <= max ? fmod(n - min, d) + min : fmod(n - min, d) + min + d;
}

double remap(double in, double in_min, double in_max, double out_min, double out_max) {
    return (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

