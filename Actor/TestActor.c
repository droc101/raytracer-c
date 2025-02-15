//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include <box2d/box2d.h>

#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Navigation.h"
#include "../Structs/Actor.h"
#include "../Structs/Vector2.h"

void TestActorSignalHandler(Actor * /*self*/, const Actor *sender, const int signal)
{
	LogDebug("Test actor got signal %d from actor %p\n", signal, sender);
}

void TestActorIdle(Actor *this, const double delta)
{
	const NavigationConfig *navigationConfig = this->extra_data;
	this->rotation += 0.01f;
	const Vector2 impulse = v2(0, navigationConfig->speed * (float)delta);
	b2Body_ApplyLinearImpulseToCenter(this->bodyId, Vector2Rotate(impulse, this->rotation), true);
}

void TestActorTargetReached(Actor *this, const double delta)
{
	const NavigationConfig *navigationConfig = this->extra_data;
	this->rotation += lerp(0, PlayerRelativeAngle(this), navigationConfig->rotationSpeed * (float)delta);
}

void CreateTestActorCollider(Actor *this, const b2WorldId worldId)
{
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = this->position;
	bodyDef.fixedRotation = true;
	bodyDef.linearDamping = 5;
	this->bodyId = b2CreateBody(worldId, &bodyDef);
	const b2Circle shape = {
		.radius = 0.25f,
	};
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR | COLLISION_GROUP_ACTOR_ENEMY;
	b2CreateCircleShape(this->bodyId, &shapeDef, &shape);
}

void TestActorInit(Actor *this, const b2WorldId worldId)
{
	CreateTestActorCollider(this, worldId);

	this->actorModel = LoadModel(MODEL("model_leafy"));
	this->actorModelTexture = TEXTURE("actor_BLOB2");
	this->SignalHandler = TestActorSignalHandler;
	ActorListenFor(this, 0);
	this->extra_data = calloc(1, sizeof(NavigationConfig));
	CheckAlloc(this->extra_data);
	NavigationConfig *navigationConfig = this->extra_data;
	navigationConfig->fov = PIf / 2;
	navigationConfig->speed = 0.0425f;
	navigationConfig->rotationSpeed = 0.1f;
	navigationConfig->directness = 0.5f;
	navigationConfig->minDistance = 1.5f;
	navigationConfig->agroDistance = 10;
	navigationConfig->deAgroDistance = 20;
	navigationConfig->agroTicks = 120;
	navigationConfig->IdleFunction = TestActorIdle;
	navigationConfig->TargetReachedFunction = TestActorTargetReached;
	navigationConfig->lastKnownTarget = this->position;
}

void TestActorUpdate(Actor *this, const double delta)
{
	this->position = b2Body_GetPosition(this->bodyId);

	NavigationStep(this, this->extra_data, delta);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TestActorDestroy(Actor *this)
{
	free(this->extra_data);
	b2DestroyBody(this->bodyId);
}
