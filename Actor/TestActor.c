//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include "../Helpers/Collision.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

void TestActorInit(Actor *this)
{
	this->solid = true;
	this->actorModel = LoadModel(MODEL("model_leafy"));
	this->actorModelTexture = TEXTURE("actor_BLOB2");
	this->actorWall = CreateWall(v2(-0.5, 0), v2(0.5, 0), actorTextures[0], 1.0, 0.0);
}

void TestActorUpdate(Actor *this, double delta)
{
	this->rotation += 0.01;

	Vector2 MoveDir = v2(0, 0.05 * delta);
	MoveDir = Vector2Rotate(MoveDir, this->rotation);

	MoveDir = Move(this->position, MoveDir, this);

	this->position = MoveDir;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TestActorDestroy(Actor *this)
{
	FreeWall(this->actorWall);
	free(this->actorWall);
}
