//
// Created by droc101 on 4/21/2024.
//

#include "Wall.h"
#include <box2d/box2d.h>
#include <box2d/types.h>
#include <math.h>
#include <string.h>
#include "../defines.h"
#include "../Helpers/Core/Error.h"

Wall *CreateWall(const Vector2 a,
				 const Vector2 b,
				 const char *texture,
				 const float uvScale,
				 const float uvOffset,
				 const b2WorldId worldId)
{
	Wall *w = malloc(sizeof(Wall));
	CheckAlloc(w);
	w->a = a;
	w->b = b;
	strncpy(w->tex, texture, 32);
	w->uvScale = uvScale;
	w->uvOffset = uvOffset;
	w->height = 1.0f;

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = a;
	w->bodyId = b2CreateBody(worldId, &bodyDef);
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;
	if (dx != 0 || dy != 0)
	{
		const b2Segment shape = {
			.point2 = {dx, dy},
		};
		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.friction = 0;
		b2CreateSegmentShape(w->bodyId, &shapeDef, &shape);
	}

	return w;
}

void WallBake(Wall *w)
{
	const float dx = w->b.x - w->a.x;
	const float dy = w->b.y - w->a.y;
	w->length = sqrtf(dx * dx + dy * dy);
	w->angle = atan2f(w->b.y - w->a.y, w->b.x - w->a.x);
}
