//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"

// Empty template functions
void ActorInit(Actor *this) {}
void ActorUpdate(Actor *this) {}
void ActorDestroy(Actor *this) {}

#include "../Actor/TestActor.h"

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

int ActorHealths[] = {
        1,
        1
};

Actor *CreateActor(Vector2 position, double rotation, int actorType) {
    Actor *actor = malloc(sizeof(Actor));
    actor->position = position;
    actor->rotation = rotation;
    actor->solid = false;
    actor->health = ActorHealths[actorType];
    actor->Init = (void (*)(void *)) ActorInitFuncs[actorType];
    actor->Update = (void (*)(void *)) ActorUpdateFuncs[actorType];
    actor->Destroy = (void (*)(void *)) ActorDestroyFuncs[actorType];
    actor->Init(actor); // kindly allow the Actor to initialize itself
    actor->actorType = actorType;
    return actor;
}

void FreeActor(Actor *actor) {
    actor->Destroy(actor);
    free(actor);
}

Wall GetTransformedWall(Actor *actor) {
    Wall wall;
    memcpy(&wall, actor->actorWall, sizeof(Wall)); // duplicate the wall struct without modifying the original

    // Rotate the wall
    wall.a = Vector2Rotate(wall.a, actor->rotation);
    wall.b = Vector2Rotate(wall.b, actor->rotation);
    // Translate the wall
    wall.a = Vector2Add(wall.a, actor->position);
    wall.b = Vector2Add(wall.b, actor->position);

    return wall;
}

