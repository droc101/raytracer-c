//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"

char *GetActorName(int actor);

char *GetActorParamName(int actor, byte param);

/**
 * Get the number of Actor types
 * @return Actor type count
 */
int GetActorTypeCount();

/**
 * Create an Actor
 * @param position Actor position
 * @param rotation Actor rotation
 * @param actorType Actor type
 * @param paramA Initial parameter A
 * @param paramB Initial parameter B
 * @param paramC Initial parameter C
 * @param paramD Initial parameter D
 * @return Initialized Actor struct
 */
Actor *CreateActor(Vector2 position,
				   double rotation,
				   int actorType,
				   byte paramA,
				   byte paramB,
				   byte paramC,
				   byte paramD);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

/**
 * Transform an actor's wall by its position and rotation
 * @param actor Actor to use
 * @param wall Wall to transform
 * @return True if the wall was transformed successfully
 * @note The @c wall variable should NOT be the actor's original wall.
 */
bool GetTransformedWall(const Actor *actor, Wall *wall);

/**
 * Add a signal to listen for
 * @param actor The actor that will listen
 * @param signal The signal to listen for
 */
void ActorListenFor(Actor *actor, const int signal);

#endif //GAME_ACTOR_H
