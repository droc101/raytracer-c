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

double wrap(double x, double min, double max) { // BUG: Sometimes returns a value outside the range, needs more testing
    if (min > max) {
        return wrap(x, max, min);
    }
    return (x >= 0 ? min : max) + fmod(x, max - min);
}

// Map a value from one range to another
double remap(double in, double in_min, double in_max, double out_min, double out_max) {
    return (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

double clampf(double x, double min, double max) {
    return x < min ? min : x > max ? max : x;
}

