//
// Created by droc101 on 11/7/2024.
//

#include "Door.h"

#include "../Structs/Wall.h"
#include "../Structs/Vector2.h"
#include "../Helpers/Collision.h"
#include "../Structs/GlobalState.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Logging.h"

typedef enum
{
    DOOR_CLOSED,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_CLOSING
} DoorState;

typedef struct DoorData
{
    DoorState state;
    ulong stateTicks;
} DoorData;

void DoorSetState(const Actor *door, DoorState state)
{
    DoorData *data = (DoorData *)door->extra_data;
    data->state = state;
    data->stateTicks = 0;
}

double GetDoorWallPos(const Actor *door)
{
    return door->actorWall->a.x;
}

void SetDoorWallPos(const Actor *door, const double pos)
{
    door->actorWall->a.x = pos;
    door->actorWall->b.x = pos + 1.0;
}

void DoorInit(Actor *this) {
    this->showShadow = false;
    this->solid = true;
    this->actorWall = CreateWall(v2(0, 0), v2(1, 0), actorTextures[11], 1, 0.0);
    this->extra_data = malloc(sizeof(DoorData));
    memset(this->extra_data, 0, sizeof(DoorData));
    DoorData *data = (DoorData *)this->extra_data;
    data->state = DOOR_CLOSED;
    data->stateTicks = 0;
}

void DoorUpdate(Actor *this) {
    DoorData *data = (DoorData *)this->extra_data;
    double wallPos;

    Wall transformedWall;
    GetTransformedWall(this, &transformedWall);
    const Vector2 wallCenter = Vector2Scale(Vector2Add(transformedWall.a, transformedWall.b), 0.5);
    bool playerCollide = CollideCylinder(wallCenter, 1.0, GetState()->level->position);

    switch (data->state)
    {
        case DOOR_CLOSED:
            if (playerCollide) {
                DoorSetState(this, DOOR_OPENING);
            }
            break;
        case DOOR_OPEN:
            if (data->stateTicks >= 60 && !playerCollide) {
                DoorSetState(this, DOOR_CLOSING);
            }
            break;
        case DOOR_OPENING:
            wallPos = data->stateTicks * (1.0 / 60.0);
            SetDoorWallPos(this, -wallPos);
            if (data->stateTicks == 60) {
                DoorSetState(this, DOOR_OPEN);
            }
            break;
        case DOOR_CLOSING:
            wallPos = data->stateTicks * (1.0 / 60.0);
            SetDoorWallPos(this, -1.0 + wallPos);
            if (data->stateTicks == 60) {
                DoorSetState(this, DOOR_CLOSED);
            }
            break;
        default:
            LogWarning("Invalid door state: %d", data->state);
            break;
    }
    data->stateTicks++;
}

void DoorDestroy(Actor *this) {
    free(this->extra_data);
    FreeWall(this->actorWall);
    free(this->actorWall);
}
