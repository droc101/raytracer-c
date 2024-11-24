//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MATHEX_H
#define GAME_MATHEX_H

/**
 * Returns the minimum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The smaller of the two numbers
 */
double min(double a, double b);

/**
 * Returns the maximum of two numbers
 * @param a Number 1
 * @param b Number 2
 * @return The larger of the two numbers
 */
double max(double a, double b);

/**
 * Wraps a number between a minimum and maximum value
 * @param n Number to wrap
 * @param min Minimum value
 * @param max Maximum value
 * @return The wrapped number
 */
double wrap(double n, double min, double max);

/**
 * Remap a number from one range to another
 * @param in Input number
 * @param in_min Input minimum
 * @param in_max Input maximum
 * @param out_min Output minimum
 * @param out_max Output maximum
 * @return Input value remapped to the output range
 */
double remap(double in, double in_min, double in_max, double out_min, double out_max);

/**
 * Linear interpolation between two numbers
 * @param a Number 1
 * @param b Number 2
 * @param f Interpolation factor
 * @return Interpolated value
 */
float lerp(float a, float b, float f);

/**
 * Clamp a number between a minimum and maximum value
 * @param x Number to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped number
 */
double clampf(double x, double min, double max);

/**
 * Convert degrees to radians
 * @param d Degrees
 */
#define degToRad(d) ((d) * 0.017453292519943295)

/**
 * Convert radians to degrees
 * @param r Radians
 */
#define radToDeg(r) ((r) * 57.29577951308232)

#endif //GAME_MATHEX_H
