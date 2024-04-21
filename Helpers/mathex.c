//
// Created by droc101 on 4/21/2024.
//

#include <math.h>
#include "../Structs/Vector2.h"
#include "../defines.h"

double min(double a, double b) // Get the minimum of two numbers
{
    return a < b ? a : b;
}

double max(double a, double b) // Get the maximum of two numbers
{
    return a > b ? a : b;
}

double wrap(double n, double min, double max) // Wrap a number between two numbers
{
    double d = max - min;
    return n == max ? n : fmod(n - min, d) + min;
}
