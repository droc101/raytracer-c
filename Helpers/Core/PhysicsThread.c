//
// Created by droc101 on 12/12/24.
//

#include "PhysicsThread.h"
#include <SDL3/SDL_thread.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "Error.h"
#include "Logging.h"
#include "Timing.h"

SDL_Thread *PhysicsThread;
SDL_Mutex *PhysicsThreadMutex;

/**
 * The function to run in the physics thread
 * @warning Only touch this when you have a lock on the mutex
 */
FixedUpdateFunction PhysicsThreadFunction;

/**
 * Whether to quit the physics thread on the next iteration
 * @warning Only touch this when you have a lock on the mutex
 */
bool PhysicsThreadPostQuit = false;

/**
 * The main function for the physics thread
 * @return 0
 */
int PhysicsThreadMain(void *)
{
	double lastFrameTime = PHYSICS_TARGET_NS_D;
	while (true)
	{
		const ulong timeStart = GetTimeNs();
		SDL_LockMutex(PhysicsThreadMutex);
		if (PhysicsThreadPostQuit)
		{
			SDL_UnlockMutex(PhysicsThreadMutex);
			return 0;
		}
		if (PhysicsThreadFunction == NULL)
		{
			SDL_UnlockMutex(PhysicsThreadMutex);
			SDL_Delay(1); // pls no spin 🥺
			continue;
		}
		// The function is copied to a local variable so we can unlock the mutex during its runtime
		const FixedUpdateFunction UpdateFunction = PhysicsThreadFunction;
		SDL_UnlockMutex(PhysicsThreadMutex);

		// delta is the portion of one "tick" that the last frame took (including idle time)
		// ticks should be around 1/60th of a second
		const double delta = lastFrameTime / PHYSICS_TARGET_NS_D;
		UpdateFunction(GetState(), delta);

		ulong timeEnd = GetTimeNs();
		ulong timeElapsed = timeEnd - timeStart;
		if (timeElapsed < PHYSICS_TARGET_NS)
		{
			SDL_DelayNS(PHYSICS_TARGET_NS - timeElapsed);
		}
		timeEnd = GetTimeNs();
		GetTimeNs();
		timeElapsed = timeEnd - timeStart;
		lastFrameTime = timeElapsed;
	}
}

void PhysicsThreadInit()
{
	PhysicsThreadFunction = NULL;
	PhysicsThreadPostQuit = false;
	PhysicsThreadMutex = SDL_CreateMutex();
	PhysicsThread = SDL_CreateThread(PhysicsThreadMain, "GamePhysics", NULL);
	if (PhysicsThread == NULL)
	{
		const char *error = SDL_GetError();
		LogError("Failed to create physics thread: %s\n", error);
		Error("Failed to create physics thread");
	}
}

void PhysicsThreadSetFunction(const FixedUpdateFunction function)
{
	SDL_LockMutex(PhysicsThreadMutex);
	PhysicsThreadFunction = function;
	SDL_UnlockMutex(PhysicsThreadMutex);
}

void PhysicsThreadTerminate()
{
	SDL_LockMutex(PhysicsThreadMutex);
	PhysicsThreadPostQuit = true;
	SDL_UnlockMutex(PhysicsThreadMutex);
	SDL_WaitThread(PhysicsThread, NULL);
	SDL_DestroyMutex(PhysicsThreadMutex);
}
