//
// Created by droc101 on 4/28/25.
//

#ifndef PHYSBOX_H
#define PHYSBOX_H
#include "box2d/id.h"
#include "../defines.h"

void PhysboxInit(Actor *this, b2WorldId worldId);
void PhysboxUpdate(Actor *this, double delta);
void PhysboxDestroy(Actor *this);

#endif //PHYSBOX_H
