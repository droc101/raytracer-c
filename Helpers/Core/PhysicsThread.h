//
// Created by droc101 on 12/12/24.
//

#ifndef PHYSICSTHREAD_H
#define PHYSICSTHREAD_H

#include "../../defines.h"

void PhysicsThreadInit();

void PhysicsThreadSetFunction(void (*function)(GlobalState *state));

void PhysicsThreadTerminate();

#endif //PHYSICSTHREAD_H
