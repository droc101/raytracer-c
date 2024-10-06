//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_TIMING_H
#define GAME_TIMING_H

#include "../defines.h"

/**
 * Set some values so that @c GetTimeNs and @c GetTimeS return time the program has been running
 */
void InitTiming();

/**
 * Get the time the program has been running in nanoseconds
 * @return Time in nanoseconds
 */
ulong GetTimeNs();

/**
 * Get the time the program has been running in seconds
 * @return Time in seconds
 */
ulong GetTimeS();

#endif //GAME_TIMING_H
