//
// Created by droc101 on 4/21/2024.
//

#include "MathEx.h"
#include <math.h>

double wrap(const double x, const double min, const double max) // NOLINT(*-no-recursion)
{
	// BUG: Sometimes returns a value outside the range, needs more testing
	if (min > max)
	{
		return wrap(x, max, min);
	}
	return (x >= 0 ? min : max) + fmod(x, max - min);
}
