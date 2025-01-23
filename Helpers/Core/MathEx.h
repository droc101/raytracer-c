//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MATHEX_H
#define GAME_MATHEX_H

#define PI 3.14159265358979323846
#define PIf 3.14159265358979323846f

/**
 * Returns the minimum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The smaller of the two numbers
 */
#define min(a, b) ((a) < (b) ? (a) : (b))

/**
 * Returns the maximum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The larger of the two numbers
 */
#define max(a, b) ((a) < (b) ? (b) : (a))

/**
 * Wraps a number between a minimum and maximum value
 * @param x Number to wrap
 * @param min Minimum value
 * @param max Maximum value
 * @return The wrapped number
 */
double wrap(double x, double min, double max);

/**
 * Remap a number from one range to another
 * @param in Input number
 * @param in_min Input minimum
 * @param in_max Input maximum
 * @param out_min Output minimum
 * @param out_max Output maximum
 * @return Input value remapped to the output range
 */
#define remap(in, in_min, in_max, out_min, out_max) \
	(((in) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

/**
 * Linear interpolation between two numbers
 * @param a Number 1
 * @param b Number 2
 * @param f Interpolation factor
 * @return Interpolated value
 */
#define lerp(a, b, f) ((a) * (1.0 - (f)) + (b) * (f))

/**
 * Clamp a number between a minimum and maximum value
 * @param val Number to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped number
 */
#define clamp(val, min, max) (val < min ? min : val > max ? max : val)

/**
 * Convert degrees to radians
 * @param d Degrees
 */
#define degToRad(d) ((d) * PI / 180)

/**
 * Convert radians to degrees
 * @param r Radians
 */
#define radToDeg(r) ((r) * 180 / PI)

#endif //GAME_MATHEX_H
