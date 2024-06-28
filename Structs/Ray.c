//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include <math.h>
#include <stdio.h>
#include "Vector2.h"
#include "Actor.h"
#include "Level.h"

// Perform a ray cast from a position and rotation into a wall. Don't forget to free the result!
inline RayCastResult Intersect(Wall wall, Vector2 from, double direction)
{
    RayCastResult rr;
    rr.Collided = false;
    rr.Distance = 0;
    if (wall.dx == 0) {
        double distance = wall.dy * (wall.a.x - from.x) / (wall.dy * cos(direction));
        if (distance > 0) {
            double y = from.y + distance * sin(direction);
            if ((y >= wall.a.y && y <= wall.b.y) || (y >= wall.b.y && y <= wall.a.y)) {
                rr.Collided = true;
                rr.Distance = distance;
                rr.CollisionPoint = vec2(wall.a.x, y);
                rr.CollisionWall = wall;
                return rr;
            }
        }
        return rr;
    }
    double distance =
            (wall.dy * (wall.a.x - from.x) - wall.dx * (wall.a.y - from.y)) /
            (wall.dy * cos(direction) - wall.dx * sin(direction));
    if (distance > 0) {
        double x = from.x + distance * cos(direction);
        if ((x >= wall.a.x && x <= wall.b.x) || (x >= wall.b.x && x <= wall.a.x)) {
            rr.Collided = true;
            rr.Distance = distance;
            rr.CollisionPoint = vec2(x, (wall.dy * (x - wall.a.x)) / wall.dx + wall.a.y);
            rr.CollisionWall = wall;
            return rr;
        }
    }
    return rr;
}

RayCastResult HitscanLevel(Level l, Vector2 pos, double angle, bool scanWalls, bool scanActors, bool alwaysCollideActors) {

    RayCastResult closestResult;
    closestResult.Collided = false;
    double closestDist = __DBL_MAX__;

    if (scanWalls) {
        for (int i = 0; i < l.staticWalls->size; i++) {
            Wall *w = SizedArrayGet(l.staticWalls, i);
            RayCastResult r = Intersect(*w, pos, angle);
            if (r.Collided) {
                if (r.Distance < closestDist) {
                    closestDist = r.Distance;
                    closestResult = r;
                }
            }
        }
    }

    if (scanActors) {
        for (int i = 0; i < l.actors->size; i++) {
            Actor *a = (Actor *) ListGet(l.actors, i);
            if (!a->solid && !alwaysCollideActors) {
                continue;
            }
            Wall w = GetTransformedWall(a);
            RayCastResult r = Intersect(w, pos, angle);
            if (r.Collided) {
                if (r.Distance < closestDist) {
                    closestDist = r.Distance;
                    closestResult = r;
                }
            }
        }
    }

    return closestResult;
}
