//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"

/**
 * Create an Actor
 * @param position Actor position
 * @param rotation Actor rotation
 * @param actorType Actor type
 * @param paramA Initial parameter A
 * @param paramB Initial parameter B
 * @param paramC Initial parameter C
 * @param paramD Initial parameter D
 * @param worldId The Box2D world within which to create the actor
 * @return Initialized Actor struct
 */
Actor *CreateActor(Vector2 position,
				   float rotation,
				   int actorType,
				   byte paramA,
				   byte paramB,
				   byte paramC,
				   byte paramD,
				   b2WorldId worldId);

/**
 * Destroy an Actor
 * @param actor actor to destroy
 */
void FreeActor(Actor *actor);

/**
 * Add a signal to listen for
 * @param actor The actor that will listen
 * @param signal The signal to listen for
 */
void ActorListenFor(Actor *actor, int signal);

#endif //GAME_ACTOR_H
