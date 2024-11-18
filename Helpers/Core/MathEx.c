//
// Created by droc101 on 4/21/2024.
//

#include "MathEx.h"
#include <math.h>

double min(const double a, const double b) // Get the minimum of two numbers
{
    return a < b ? a : b;
}

double max(const double a, const double b) // Get the maximum of two numbers
{
    return a > b ? a : b;
}

double wrap(const double x, const double min, const double max) // NOLINT(*-no-recursion)
{
    // BUG: Sometimes returns a value outside the range, needs more testing
    if (min > max)
    {
        return wrap(x, max, min);
    }
    return (x >= 0 ? min : max) + fmod(x, max - min);
}

// Map a value from one range to another
double remap(const double in, const double in_min, const double in_max, const double out_min, const double out_max)
{
    return (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float lerp(const float a, const float b, const float f)
{
    return a * (1.0 - f) + b * f;
}
