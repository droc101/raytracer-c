//
// Created by noah on 2/10/25.
//

#include "Collision.h"
#include <box2d/box2d.h>
#include <box2d/types.h>

bool GetSensorState(const b2WorldId worldId, const uint sensorShapeIdIndex, const bool currentState)
{
	const b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);
	if (currentState)
	{
		for (int i = 0; i < sensorEvents.endCount; i++)
		{
			const b2SensorEndTouchEvent event = sensorEvents.endEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				return false;
			}
		}
	} else
	{
		for (int i = 0; i < sensorEvents.beginCount; i++)
		{
			const b2SensorBeginTouchEvent event = sensorEvents.beginEvents[i];
			if (event.sensorShapeId.index1 == sensorShapeIdIndex)
			{
				return true;
			}
		}
	}

	return currentState;
}
