//
// Created by droc101 on 12/28/24.
//

#include "FrameBenchmark.h"

#include <limits.h>

#include "../defines.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/Timing.h"

bool benchRunning = false;
ulong benchStartTime;
ulong benchFrameCount;
ulong highestFrameNs;
ulong lowestFrameNs;

ulong benchFrameStartTime;

void BenchStart()
{
	benchRunning = true;
	benchStartTime = GetTimeNs();
	benchFrameCount = 0;
	highestFrameNs = 0;
	lowestFrameNs = ULONG_MAX;
	BenchFrameStart();
	LogDebug("Benchmark started\n");
}

void BenchFrameStart()
{
	if (!benchRunning)
	{
		return;
	}
	benchFrameStartTime = GetTimeNs();
}

void BenchFrameEnd()
{
	if (!benchRunning)
	{
		return;
	}
	const ulong frameTime = GetTimeNs() - benchFrameStartTime;
	if (frameTime < lowestFrameNs)
	{
		lowestFrameNs = frameTime;
	}
	if (frameTime > highestFrameNs)
	{
		highestFrameNs = frameTime;
	}
	benchFrameCount++;
}

void BenchFinish()
{
	benchRunning = false;
	const ulong endTime = GetTimeNs();
	const ulong totalTime = endTime - benchStartTime;
	double avgFrameTime = (double)totalTime / benchFrameCount;
	const double avgFps = 1.0 / (avgFrameTime / 1000000000.0);

	avgFrameTime /= 1000000.0;

	const double lowestFrameTime = (double)lowestFrameNs / 1000000.0;
	const double highestFrameTime = (double)highestFrameNs / 1000000.0;

	LogDebug("Benchmark finished\n");
	LogDebug("Average frame time: %f ms\n", avgFrameTime);
	LogDebug("Average FPS: %f\n", avgFps);
	LogDebug("Lowest frame time: %f ms\n", lowestFrameTime);
	LogDebug("Highest frame time: %f ms\n", highestFrameTime);

}

void BenchToggle()
{
	if (benchRunning)
	{
		BenchFinish();
	} else
	{
		BenchStart();
	}
}
