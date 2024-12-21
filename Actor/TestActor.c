//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include "../Assets/AssetReader.h"
#include "../Assets/Assets.h"
#include "../Helpers/Collision.h"
#include "../Helpers/CommonAssets.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

Model *leafyModel = NULL;

void TestActorInit(Actor *this)
{
	if (leafyModel == NULL)
	{
		leafyModel = LoadModel(gzobj_model_leafy);
	}
	this->solid = true;
	this->actorModel = leafyModel;
	this->actorModelTexture = gztex_actor_BLOB2;
	this->actorWall = CreateWall(v2(-0.5, 0), v2(0.5, 0), actorTextures[0], 1.0, 0.0);
}

void TestActorUpdate(Actor *this)
{
	this->rotation += 0.01;

	Vector2 MoveDir = v2(0, 0.05);
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
