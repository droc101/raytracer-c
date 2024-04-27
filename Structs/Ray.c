//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include <math.h>
#include "Vector2.h"
#include "Actor.h"

// Perform a ray cast from a position and rotation into a wall. Don't forget to free the result!
RayCastResult Intersect(Wall wall, Vector2 from, double direction)
{
    Vector2 vertA = wall.a;
    Vector2 vertB = wall.b;
    Vector2 wallVector = vec2(vertB.x - vertA.x, vertB.y - vertA.y);
    Vector2 rayVector = vec2(cos(direction), sin(direction));
    double denominator = (rayVector.x * wallVector.y) - (rayVector.y * wallVector.x);
    if (denominator == 0)
    {
        RayCastResult rr;
        rr.Collided = false;
        return rr; // bail out so we dont div/0
    }
    double t = ((vertA.x - from.x) * wallVector.y - (vertA.y - from.y) * wallVector.x) / denominator;
    double u = ((vertA.x - from.x) * rayVector.y - (vertA.y - from.y) * rayVector.x) / denominator;
    if (t >= 0 && u >= 0 && u <= 1)
    {
        double intersectionX = from.x + t * rayVector.x;
        double intersectionY = from.y + t * rayVector.y;
        Vector2 colPoint = vec2(intersectionX, intersectionY);
        RayCastResult rr;
        rr.Collided = true;
        rr.CollisionPoint = colPoint;
        rr.CollisionWall = wall;
        return rr;
    }
    RayCastResult rr;
    rr.Collided = false;
    return rr; // no intersection
}

RayCastResult HitscanLevel(Level l, Vector2 pos, double angle, bool scanWalls, bool scanActors, bool alwaysCollideActors) {

    RayCastResult closestResult;
    closestResult.Collided = false;
    double closestDist = 999999;

    if (scanWalls) {
        for (int i = 0; i < l.walls->size; i++) {
            Wall *w = (Wall *) ListGet(l.walls, i);
            RayCastResult r = Intersect(*w, pos, angle);
            if (r.Collided) {
                double dist = Vector2Distance(l.position, r.CollisionPoint);
                if (dist < closestDist) {
                    closestDist = dist;
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
                double dist = Vector2Distance(l.position, r.CollisionPoint);
                if (dist < closestDist) {
                    closestDist = dist;
                    closestResult = r;
                }
            }
        }
    }

    return closestResult;
}
