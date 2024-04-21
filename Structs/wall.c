//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "wall.h"
#include <math.h>
#include "../Helpers/drawing.h"

Wall CreateWall(Vector2 a, Vector2 b, uint tex) {
    Wall w;
    w.a = a;
    w.b = b;
    w.tex = ToSDLTexture((const unsigned char *) tex_level_bricks);
    return w;
}

double WallGetLength(Wall w) {
    return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(Wall w) {
    return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}
