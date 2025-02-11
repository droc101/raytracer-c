//
// Created by droc101 on 1/18/25.
//

#ifndef TRIGGER_H
#define TRIGGER_H

#include "Vector2.h"

/**
 * Create a trigger
 * @param position The position of the trigger
 * @param extents The extents of the trigger (half in each direction)
 * @param rotation The rotation of the trigger
 * @param command The command to execute when the trigger is activated
 * @param flags The flags of the trigger. See @c TriggerFlags for a list of flags
 * @param worldId The Box2D world within which to create the trigger
 * @return The created trigger
 */
Trigger *CreateTrigger(Vector2 position,
					   Vector2 extents,
					   float rotation,
					   const char *command,
					   uint flags,
					   b2WorldId worldId);

/**
 * Check if a player is colliding with a trigger
 * @param trigger The trigger to check
 * @return Whether the player is colliding with the trigger
 */
bool CheckTriggerCollision(Trigger *trigger);

#endif //TRIGGER_H
