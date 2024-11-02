//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include <math.h>
#include "Vector2.h"
#include "Actor.h"

// Perform a ray cast from a position and rotation into a wall. Don't forget to free the result!
RayCastResult Intersect(const Wall wall, const Vector2 from, const double direction)
{
    RayCastResult rr;
    rr.Collided = false;
    rr.Distance = 0;
    if (wall.dx == 0)
    {
        const double distance = wall.dy * (wall.a.x - from.x) / (wall.dy * cos(direction));
        if (distance > 0)
        {
            const double y = from.y + distance * sin(direction);
            if ((y >= wall.a.y && y <= wall.b.y) || (y >= wall.b.y && y <= wall.a.y))
            {
                rr.Collided = true;
                rr.Distance = distance;
                rr.CollisionPoint = v2(wall.a.x, y);
                rr.CollisionWall = wall;
                return rr;
            }
        }
        return rr;
    }
    const double distance =
            (wall.dy * (wall.a.x - from.x) - wall.dx * (wall.a.y - from.y)) /
            (wall.dy * cos(direction) - wall.dx * sin(direction));
    if (distance > 0)
    {
        const double x = from.x + distance * cos(direction);
        if ((x >= wall.a.x && x <= wall.b.x) || (x >= wall.b.x && x <= wall.a.x))
        {
            rr.Collided = true;
            rr.Distance = distance;
            rr.CollisionPoint = v2(x, (wall.dy * (x - wall.a.x)) / wall.dx + wall.a.y);
            rr.CollisionWall = wall;
            return rr;
        }
    }
    return rr;
}
