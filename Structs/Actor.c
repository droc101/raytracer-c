//
// Created by droc101 on 4/22/2024.
//

#include "Actor.h"
#include "Wall.h"

// Empty template functions
void ActorInit(Actor *this)
{
}

void ActorUpdate(Actor *this)
{
}

void ActorDestroy(Actor *this)
{
}

#include "../Actor/TestActor.h"
#include "../Actor/Coin.h"
#include "../Actor/Goal.h"

void (*ActorInitFuncs[])(Actor *) = {
    ActorInit,
    TestActorInit,
    CoinInit,
    GoalInit
};

void (*ActorUpdateFuncs[])(Actor *) = {
    ActorUpdate,
    TestActorUpdate,
    CoinUpdate,
    GoalUpdate
};

void (*ActorDestroyFuncs[])(Actor *) = {
    ActorDestroy,
    TestActorDestroy,
    CoinDestroy,
    GoalDestroy
};

int ActorHealths[] = {
    1,
    1,
    1,
    1
};

char *ActorNames[] = {
    "NullActor",
    "TestActor",
    "Coin",
    "Goal"
};

// Array of actor parameter names
// Each actor type has 4 parameters
char ActorParamNames[][4][16] = {
    {"N/A", "N/A", "N/A", "N/A"},
    {"N/A", "N/A", "N/A", "N/A"},
    {"Anim Frame", "Blue Coin?", "N/A", "N/A"},
    {"N/A", "N/A", "N/A", "N/A"},
};

char *GetActorName(int actor) {
    const int actorNameCount = sizeof(ActorNames) / sizeof(char*);
    if (actor > actorNameCount) {
        return "Invalid!";
    }
    return ActorNames[actor];
}

char *GetActorParamName(int actor, byte param) {
    const int actorNameCount = sizeof(ActorNames) / sizeof(char*);
    if (actor > actorNameCount) {
        return "Invalid!";
    }
    return ActorParamNames[actor][param];
}

int GetActorTypeCount()
{
    return sizeof(ActorInitFuncs) / sizeof(void *);
}

Actor *CreateActor(const Vector2 position, const double rotation, const int actorType, const byte paramA, const byte paramB, const byte paramC, const byte paramD)
{
    Actor *actor = malloc(sizeof(Actor));
    actor->actorWall = NULLPTR;
    actor->position = position;
    actor->rotation = rotation;
    actor->solid = false;
    actor->health = ActorHealths[actorType];
    actor->paramA = paramA;
    actor->paramB = paramB;
    actor->paramC = paramC;
    actor->paramD = paramD;
    actor->yPosition = 0.0f;
    actor->showShadow = true;
    actor->shadowSize = 1.0f;
    actor->Init = (void (*)(void *)) ActorInitFuncs[actorType];
    actor->Update = (void (*)(void *)) ActorUpdateFuncs[actorType];
    actor->Destroy = (void (*)(void *)) ActorDestroyFuncs[actorType];
    actor->Init(actor); // kindly allow the Actor to initialize itself
    actor->actorType = actorType;
    return actor;
}

void FreeActor(Actor *actor)
{
    actor->Destroy(actor);
    free(actor);
}

bool GetTransformedWall(const Actor *actor, Wall *wall)
{
    if (actor->actorWall == NULLPTR)
    {
        return false;
    }

    memcpy(wall, actor->actorWall, sizeof(Wall)); // duplicate the wall struct without modifying the original

    // Rotate the wall
    wall->a = Vector2Rotate(wall->a, actor->rotation);
    wall->b = Vector2Rotate(wall->b, actor->rotation);
    // Translate the wall
    wall->a = Vector2Add(wall->a, actor->position);
    wall->b = Vector2Add(wall->b, actor->position);

    WallBake(wall);

    return true;
}
