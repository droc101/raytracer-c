//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "Wall.h"
#include <math.h>
#include "../Helpers/Drawing.h"
#include "Vector2.h"

const uint *wallTextures[] = {
        gztex_level_bricks,
        gztex_level_cross,
        gztex_level_wall2,
        gztex_actor_iq,
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

Vector2 PushPointOutOfWallHitbox(Wall w, Vector2 movementVector) {
    // https://www.desmos.com/geometry/uz6y8x3emg
    // TODO: Implement this function (unfortunately, the math isn't mathing)
    // Push the point out of the wall hitbox perpendicular to the wall
    // The wall is the line between a and b
    // The point is the point to push out of the wall
    // Use the originPoint to determine which side of the wall to push the point out of
    Vector2 pos = Vector2Normalize(vec2o(w.b.y - w.a.y + movementVector.x, -w.b.x + w.a.x + movementVector.y, movementVector.x, movementVector.y));
    if (Vector2Dot(movementVector, pos) < 0) {
        return Vector2RemoveOrigin(Vector2Scale(pos, WALL_HITBOX_EXTENTS));
    }
    return Vector2RemoveOrigin(Vector2Scale(pos, -WALL_HITBOX_EXTENTS));
}

