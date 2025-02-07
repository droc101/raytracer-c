//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"

#include <box2d/box2d.h>

#include "../Helpers/Collision.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Logging.h"
#include "../Structs/Actor.h"
#include "../Structs/Vector2.h"

void TestActorSignalHandler(Actor * /*self*/, const Actor *sender, const int signal)
{
	LogDebug("Test actor got signal %d from actor %p\n", signal, sender);
}

void TestActorCreateCollider(Actor *this, const b2WorldId worldId)
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
	shapeDef.density = 32768;
	shapeDef.filter.categoryBits = COLLISION_GROUP_ACTOR;
	b2CreateCircleShape(this->bodyId, &shapeDef, &shape);
}

void TestActorInit(Actor *this, const b2WorldId worldId)
{
	TestActorCreateCollider(this, worldId);

	this->actorModel = LoadModel(MODEL("model_leafy"));
	this->actorModelTexture = TEXTURE("actor_BLOB2");
	this->SignalHandler = TestActorSignalHandler;
	ActorListenFor(this, 0);
}

void TestActorUpdate(Actor *this, const double delta)
{
	this->position = b2Body_GetPosition(this->bodyId);
	this->rotation += 0.01f;

	b2Body_ApplyLinearImpulseToCenter(this->bodyId,
									  Vector2Rotate((Vector2){.y = 1638 * (float)delta}, this->rotation),
									  true);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TestActorDestroy(Actor *this)
{
	b2DestroyBody(this->bodyId);
}
