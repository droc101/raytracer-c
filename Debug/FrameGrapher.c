//
// Created by droc101 on 4/24/24.
//

#include "FrameGrapher.h"
#include <limits.h>
#include <stdio.h>
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Core/Timing.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/Vector2.h"

double framerates[FRAMEGRAPH_HISTORY_SIZE] = {0};
long framegraphLastUpdateTime = LONG_MIN;

void FG_PushIntoArray(const double value)
{
	for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE - 1; i++)
	{
		framerates[i] = framerates[i + 1];
	}
	framerates[FRAMEGRAPH_HISTORY_SIZE - 1] = value;
}

void FrameGraphUpdate(const ulong ns)
{
	// If it's not time to update the graph, return
	if (GetTimeMs() - framegraphLastUpdateTime < FRAMEGRAPH_INTERVAL)
	{
		return;
	}

	FG_PushIntoArray(ns == 0 ? 1 : ns);
	framegraphLastUpdateTime = GetTimeMs();
}

void FrameGraphDraw()
{
#ifdef FRAMEGRAPH_ENABLE
#ifndef FRAMEGRAPH_FPS_ONLY
	// Draw a background for the graph
	SetColorUint(0x80000000);
	DrawRect(0,
			 WindowHeight() - FRAMEGRAPH_THRESHOLD_GOOD * 2 * FRAMEGRAPH_V_SCALE - 20,
			 FRAMEGRAPH_H_SCALE * FRAMEGRAPH_HISTORY_SIZE + 10,
			 WindowHeight() - 10);

	// Draw a line at the bottom of the graph
	SetColorUint(0x80808080);
	DrawLine(v2(10, WindowHeight() - 10), v2(FRAMEGRAPH_H_SCALE * FRAMEGRAPH_HISTORY_SIZE, WindowHeight() - 10), 2);

	// Draw a line at the target framerate
	SetColorUint(0x80808080);
	DrawLine(v2(10, WindowHeight() - 10 - FRAMEGRAPH_THRESHOLD_GOOD * FRAMEGRAPH_V_SCALE),
			 v2(FRAMEGRAPH_H_SCALE * FRAMEGRAPH_HISTORY_SIZE,
				WindowHeight() - 10 - FRAMEGRAPH_THRESHOLD_GOOD * FRAMEGRAPH_V_SCALE),
			 2);
	FontDrawString(v2(10, WindowHeight() - 10 - FRAMEGRAPH_THRESHOLD_GOOD * FRAMEGRAPH_V_SCALE - 6),
				   "Target",
				   12,
				   0xff00ffff,
				   true);

	uint lineColor = 0;
	// Draw a line graph of all the frame rates/times
	for (int i = FRAMEGRAPH_HISTORY_SIZE - 2; i >= 0; i--)
	{
		if (framerates[i] == 0)
		{
			break;
		}

		const double ns = framerates[i];
		const double nextNs = framerates[i + 1];

		double f = 1000000000.0 / ns;
		double nextF = 1000000000.0 / nextNs;

#ifdef FRAMEGRAPH_SHOW_LINEAR_TIME_GRAPH
		double nsRemapped = remap(ns, 0, FRAMEGRAPH_NSPF, 0, FRAMEGRAPH_THRESHOLD_GOOD);
		double nextNsRemapped = remap(nextNs, 0, FRAMEGRAPH_NSPF, 0, FRAMEGRAPH_THRESHOLD_GOOD);
#endif


#ifdef FRAMEGRAPH_ENABLE_CAPPING
		if (f > FRAMEGRAPH_THRESHOLD_GOOD * 2)
		{
			f = FRAMEGRAPH_THRESHOLD_GOOD * 2;
		}
		if (nextF > FRAMEGRAPH_THRESHOLD_GOOD * 2)
		{
			nextF = FRAMEGRAPH_THRESHOLD_GOOD * 2;
		}
#ifdef FRAMEGRAPH_SHOW_LINEAR_TIME_GRAPH
		if (nsRemapped > FRAMEGRAPH_THRESHOLD_GOOD * 2)
		{
			nsRemapped = FRAMEGRAPH_THRESHOLD_GOOD * 2;
		}
		if (nextNsRemapped > FRAMEGRAPH_THRESHOLD_GOOD * 2)
		{
			nextNsRemapped = FRAMEGRAPH_THRESHOLD_GOOD * 2;
		}
#endif
#endif


		// first line for fps
		const double x1 = (double)i * FRAMEGRAPH_H_SCALE + 10;
		double y1 = (double)WindowHeight() - f * FRAMEGRAPH_V_SCALE - 10;
		const double x2 = (double)(i + 1) * FRAMEGRAPH_H_SCALE + 10;
		double y2 = (double)WindowHeight() - nextF * FRAMEGRAPH_V_SCALE - 10;

		if (f > FRAMEGRAPH_THRESHOLD_GOOD)
		{
			lineColor = 0xff00ff00;
		} else if (f < FRAMEGRAPH_THRESHOLD_BAD)
		{
			lineColor = 0xffff0000;
		} else
		{
			lineColor = 0xffff8000;
		}
		SetColorUint(lineColor);
		DrawLine(v2(x1, y1), v2(x2, y2), 2);
#ifdef FRAMEGRAPH_SHOW_LINEAR_TIME_GRAPH
		// 2nd line for frame time
		y1 = (double)WindowHeight() - nsRemapped * FRAMEGRAPH_V_SCALE - 10;
		y2 = (double)WindowHeight() - nextNsRemapped * FRAMEGRAPH_V_SCALE - 10;
		SetColorUint((lineColor & 0x00ffffff) | 0x80000000); // set the alpha to 50% of the line color
		DrawLine(v2(x1, y1), v2(x2, y2), 2);
#endif
	}
#else
	uint lineColor;
#endif
	const double currentNs = framerates[FRAMEGRAPH_HISTORY_SIZE - 1];
	const double currentF = 1000000000.0 / currentNs;
	const double currentMs = currentNs / 1000000.0;

	if (currentF > FRAMEGRAPH_THRESHOLD_GOOD)
	{
		lineColor = 0xff00ff00;
	} else if (currentF < FRAMEGRAPH_THRESHOLD_BAD)
	{
		lineColor = 0xffff0000;
	} else
	{
		lineColor = 0xffff8000;
	}

	// Draw the current framerate
	SetColorUint(0xffffffff);
	char fps[40];
	sprintf(fps, "FPS: %.2f\nMS: %2.2f", currentF, currentMs);
	FontDrawString(v2(12, WindowHeight() - 8 - 38), fps, 16, 0xff000000, true);
	FontDrawString(v2(10, WindowHeight() - 10 - 38), fps, 16, lineColor, true);

#endif
}
