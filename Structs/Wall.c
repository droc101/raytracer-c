//
// Created by droc101 on 4/21/2024.
//

#include "Wall.h"
#include <math.h>
#include "../defines.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Error.h"

Wall *CreateWall(const Vector2 a, const Vector2 b, const char *tex, const float uvScale, const float uvOffset)
{
	Wall *w = malloc(sizeof(Wall));
	chk_malloc(w);
	w->a = a;
	w->b = b;
	w->tex = tex;
	w->texId = FindWallTextureIndex(tex);
	w->uvScale = uvScale;
	w->uvOffset = uvOffset;
	w->height = 1.0f;
	return w;
}

void FreeWall(Wall * /*w*/)
{
	// no longer need to free the texture
}

double WallGetLength(const Wall w)
{
	return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(const Wall w)
{
	return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}

double WallBake(Wall *w)
{
	w->length = WallGetLength(*w);
	w->angle = WallGetAngle(*w);
	w->dx = w->a.x - w->b.x;
	w->dy = w->a.y - w->b.y;
	return w->length;
}
