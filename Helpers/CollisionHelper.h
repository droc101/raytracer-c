//
// Created by Noah on 4/28/2024.
//

#ifndef GAME_COLLISIONHELPER_H
#define GAME_COLLISIONHELPER_H

#define abx(wall, position) ((wall.a.x + WALL_HITBOX_EXTENTS <= position.x || wall.a.x - WALL_HITBOX_EXTENTS <= position.x) && (wall.b.x + WALL_HITBOX_EXTENTS >= position.x || wall.b.x - WALL_HITBOX_EXTENTS >= position.x))
#define bax(wall, position) ((wall.b.x + WALL_HITBOX_EXTENTS <= position.x || wall.b.x - WALL_HITBOX_EXTENTS <= position.x) && (wall.a.x + WALL_HITBOX_EXTENTS >= position.x || wall.a.x - WALL_HITBOX_EXTENTS >= position.x))
#define aby(wall, position) ((wall.a.y + WALL_HITBOX_EXTENTS <= position.y || wall.a.y - WALL_HITBOX_EXTENTS <= position.y) && (wall.b.y + WALL_HITBOX_EXTENTS >= position.y || wall.b.y - WALL_HITBOX_EXTENTS >= position.y))
#define bay(wall, position) ((wall.b.y + WALL_HITBOX_EXTENTS <= position.y || wall.b.y - WALL_HITBOX_EXTENTS <= position.y) && (wall.a.y + WALL_HITBOX_EXTENTS >= position.y || wall.a.y - WALL_HITBOX_EXTENTS >= position.y))

#define nearWall(wall, position) ((abx(wall, position) && aby(wall, position)) || (abx(wall, position) && bay(wall, position)) || (bax(wall, position) && aby(wall, position)) || (bax(wall, position) && bay(wall, position)))

#define isWallStraight(wall) (wall.a.x == wall.b.x || wall.a.y == wall.b.y)

#define dy(wall, position) (fabs((position.x - wall.a.x) * ((wall.b.y - wall.a.y) / (wall.b.x - wall.a.x)) + wall.a.y - position.y))
#define dx(wall, position) (fabs((position.y - wall.a.y) * ((wall.b.x - wall.a.x) / (wall.b.y - wall.a.y)) + wall.a.x - position.x))

// TODO: make this be inHitbox, not notInHitbox
// TODO: You better make sure not to hardcode the player's movement speed in here
#define notInHitbox(wall, position) (dy(wall, position) > WALL_HITBOX_EXTENTS && dx(wall, position) > WALL_HITBOX_EXTENTS && sqrt(pow(dx(wall, position), 2) + pow(dy(wall, position), 2)) > MOVE_SPEED)


#endif //GAME_COLLISIONHELPER_H
