//
// Created by droc101 on 4/21/2024.
//

#include "MathEx.h"
#include <math.h>

int wrapi(int x, int min, int max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + x % max - min;
}

float wrapf(float x, float min, float max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + fmodf(x, max - min);
}

double wrapd(double x, double min, double max)
{
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + fmod(x, max - min);
}
