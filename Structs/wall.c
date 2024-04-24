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
        tex_level_wall2,
        tex_actor_iq,
};

Wall *CreateWall(Vector2 a, Vector2 b, uint tex) {
    Wall *w = malloc(sizeof(Wall));
    w->a = a;
    w->b = b;
    w->tex = ToSDLTexture((const unsigned char *) wallTextures[tex], FILTER_NEAREST);
    return w;
}

void FreeWall(Wall *w) {
    SDL_DestroyTexture(w->tex);
}

double WallGetLength(Wall w) {
    return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(Wall w) {
    return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}

Vector2 PushPointOutOfWallHitbox(Wall w, Vector2 point, Vector2 originPoint) {
    // TODO: Implement this function (unfortunately, the math isn't mathing)
    // Push the point out of the wall hitbox perpendicular to the wall
    // The wall is the line between a and b
    // The point is the point to push out of the wall
    // Use the originPoint to determine which side of the wall to push the point out of

    return originPoint;
}

