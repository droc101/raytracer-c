//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "wall.h"
#include <math.h>

const uint *texIds[] = {
        tex_level_bricks,
        tex_level_cross,
        tex_level_wall2
};

Wall CreateWall(Vector2 a, Vector2 b, uint tex) {
    Wall w;
    w.a = a;
    w.b = b;
    w.tex = texIds[tex];
    return w;
}

double WallGetLength(Wall w) {
    return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(Wall w) {
    return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}
