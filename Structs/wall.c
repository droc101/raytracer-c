//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "wall.h"
#include <math.h>
#include "../Helpers/drawing.h"
#include "Vector2.h"

const uint *wallTextures[] = {
        tex_level_bricks,
        tex_level_cross,
        tex_level_wall2
};

Wall *CreateWall(Vector2 a, Vector2 b, uint tex) {
    Wall *w = malloc(sizeof(Wall));
    w->a = a;
    w->b = b;
    w->tex = ToSDLTexture((const unsigned char *) wallTextures[tex], "0");
    return w;
}

void FreeWall(Wall w) {
    SDL_DestroyTexture(w.tex);
}

double WallGetLength(Wall w) {
    return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(Wall w) {
    return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}

Vector2 PushPointOutOfWallHitbox(Wall w, Vector2 point) {
    // Push the point out of the wall hitbox perpendicularly to the wall
    double angle = WallGetAngle(w) + PI / 2;
    Vector2 offset = Vector2Scale(Vector2FromAngle(angle), WALL_HITBOX_EXTENTS);

    // Check which side of the wall the point is on, prevent it from being pushed out the wrong way
    Vector2 wallDir = Vector2Normalize(Vector2Sub(w.b, w.a));
    Vector2 pointDir = Vector2Normalize(Vector2Sub(point, w.a));
    if (Vector2Dot(wallDir, pointDir) < 0) {
        offset = Vector2Scale(offset, -1);
    }

    return Vector2Add(point, offset);
}

