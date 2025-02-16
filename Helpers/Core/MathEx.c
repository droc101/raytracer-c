//
// Created by droc101 on 4/21/2024.
//

#include "MathEx.h"
#include <math.h>

int wrapi(const int x, const int min, const int max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + x % max - min;
}

float wrapf(const float x, const float min, const float max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + fmodf(x, max - min);
}

double wrapd(const double x, const double min, const double max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + fmod(x, max - min);
}
