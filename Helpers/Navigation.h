//
// Created by noah on 2/8/25.
//

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "../defines.h"

typedef struct NavigationConfig NavigationConfig;

/// TODO Document
struct NavigationConfig
{
	float fov;
	float speed;
	float rotationSpeed;
	float directness;
	float minDistance;
	float agroDistance;
	float deAgroDistance;
	uint agroTicks;
	ActorIdleFunction IdleFunction;
	ActorTargetReachedFunction TargetReachedFunction;

	double agroTicksRemaining;
	Vector2 lastKnownTarget;
	float directionModifier;
	double ticksUntilDirectionChange;
};

float PlayerRelativeAngle(const Actor *actor);

float RaycastCallback(b2ShapeId shapeId, Vector2 point, Vector2 normal, float fraction, void *raycastHit);

bool IsPlayerVisible(const Actor *actor, NavigationConfig navigationConfig);

void NavigationStep(Actor *actor, NavigationConfig *navigationConfig, double delta);

#endif //NAVIGATION_H
