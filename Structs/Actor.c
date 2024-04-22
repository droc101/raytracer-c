//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include "../defines.h"
#include "../Structs/Vector2.h"
#include "wall.h"

// Empty template functions
void ActorInit(Actor *this) {}
void ActorUpdate(Actor *this) {}
void ActorDestroy(Actor *this) {}

#include "../actor/TestActor.h"

void(*ActorInitFuncs[])(Actor *) = {
        ActorInit,
        TestActorInit
};
void(*ActorUpdateFuncs[])(Actor *) = {
        ActorUpdate,
        TestActorUpdate
};
void(*ActorDestroyFuncs[])(Actor *) = {
        ActorDestroy,
        TestActorDestroy
};

Actor *CreateActor(Vector2 position, double rotation, int actorType) {
    Actor *actor = malloc(sizeof(Actor));
    actor->position = position;
    actor->rotation = rotation;
    actor->solid = false;
    actor->actorWall = *CreateWall(vec2(-1, 0), vec2(1, 0), 0);
    actor->Init = (void (*)(void *)) ActorInitFuncs[actorType];
    actor->Update = (void (*)(void *)) ActorUpdateFuncs[actorType];
    actor->Destroy = (void (*)(void *)) ActorDestroyFuncs[actorType];
    actor->Init(actor); // kindly allow the actor to initialize itself
    return actor;
}

void FreeActor(Actor *actor) {
    actor->Destroy(actor);
    free(actor);
}

Wall GetTransformedWall(Actor *actor) {
    Wall wall;
    memcpy(&wall, &actor->actorWall, sizeof(Wall)); // duplicate the wall struct without modifying the original

    // Rotate the wall
    wall.a = Vector2Rotated(wall.a, actor->rotation);
    wall.b = Vector2Rotated(wall.b, actor->rotation);
    // Translate the wall
    wall.a = Vector2Add(wall.a, actor->position);
    wall.b = Vector2Add(wall.b, actor->position);

    return wall;
}

