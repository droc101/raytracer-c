//
// Created by droc101 on 4/26/2024.
//

#include "Timing.h"
#include <time.h>

ulong GetTimeNs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}
