//
// Created by droc101 on 1/18/25.
//

#include "Trigger.h"
#include <string.h>
#include "../Helpers/Core/Error.h"
#include "Vector2.h"

Trigger *CreateTrigger(const Vector2 pos,
					   const Vector2 extents,
					   const double rot,
					   const char *command,
					   const uint flags)
{
	Trigger *t = malloc(sizeof(Trigger));
	CheckAlloc(t);
	t->position = pos;
	t->extents = extents;
	t->rotation = rot;
	t->flags = flags;
	strncpy(t->command, command, 63);

	return t;
}

bool CheckTriggerCollision(const Trigger *t, const Player *p)
{
	const Vector2 rotatedPlayerPos = Vector2Rotate(Vector2Sub(p->pos, t->position), -t->rotation);
	const Vector2 halfExtents = Vector2Scale(t->extents, 0.5);
	return rotatedPlayerPos.x >= -halfExtents.x &&
		   rotatedPlayerPos.x <= halfExtents.x &&
		   rotatedPlayerPos.y >= -halfExtents.y &&
		   rotatedPlayerPos.y <= halfExtents.y;
}
