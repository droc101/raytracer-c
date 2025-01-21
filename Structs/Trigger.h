//
// Created by droc101 on 1/18/25.
//

#ifndef TRIGGER_H
#define TRIGGER_H
#include "Vector2.h"

/**
 * Create a trigger
 * @param pos The position of the trigger
 * @param extents The extents of the trigger (half in each direction)
 * @param rot The rotation of the trigger
 * @param command The command to execute when the trigger is activated
 * @param flags
 * @return The created trigger
 */
Trigger *CreateTrigger(Vector2 pos, Vector2 extents, double rot, const char *command, uint flags);

/**
 * Check if a player is colliding with a trigger
 * @param t The trigger to check
 * @param p The player to check
 * @return Whether the player is colliding with the trigger
 */
bool CheckTriggerCollision(const Trigger *t, const Player *p);

#endif //TRIGGER_H
