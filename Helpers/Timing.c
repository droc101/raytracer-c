//
// Created by droc101 on 4/26/2024.
//

#include "Timing.h"
#include <time.h>

static ulong StartTimeNS;
static ulong StartTimeS;

void InitTimers() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    StartTimeNS = ts.tv_sec * 1000000000 + ts.tv_nsec;
    StartTimeS = ts.tv_sec;
}

ulong GetTimeNs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec - StartTimeNS;
}

ulong GetTimeS() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec - StartTimeS;
}
