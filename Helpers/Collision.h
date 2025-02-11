//
// Created by noah on 2/10/25.
//

#ifndef COLLISION_H
#define COLLISION_H

#include "../defines.h"

/**
 * Get the state of a sensor. The current state of the sensor must be passed into this function because Box2D sensors
 * only provide events when collision starts or stops
 * @param worldId The Box2D world that the sensor is in
 * @param sensorShapeIdIndex The @c index1 member of the sensor shape ID
 * @param currentState A boolean for the current state of the sensor
 * @return The new state of the sensor. True if something is colliding with it, false if nothing is colliding with it
 */
bool GetSensorState(b2WorldId worldId, uint sensorShapeIdIndex, bool currentState);

#endif //COLLISION_H
