//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_MATHEX_H
#define GAME_MATHEX_H

double min(double a, double b);
double max(double a, double b);
double wrap(double n, double min, double max);
double remap(double in, double in_min, double in_max, double out_min, double out_max);
float lerp(float a, float b, float f);

#define degToRad(d) (d * 0.017453292519943295)
#define radToDeg(r) (r * 57.29577951308232)

#endif //GAME_MATHEX_H
