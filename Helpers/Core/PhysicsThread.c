//
// Created by droc101 on 12/12/24.
//

#include "PhysicsThread.h"
#include <SDL_thread.h>

#include "../../defines.h"
#include "../../Structs/GlobalState.h"
#include "Error.h"
#include "Logging.h"

SDL_Thread *PhysicsThread;
SDL_mutex *PhysicsThreadMutex;

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
	while (true)
	{
		const ulong timeStart = SDL_GetTicks64();
		SDL_LockMutex(PhysicsThreadMutex);
		if (PhysicsThreadPostQuit)
		{
			SDL_UnlockMutex(PhysicsThreadMutex);
			return 0;
		}
		if (PhysicsThreadFunction == NULL)
		{
			SDL_UnlockMutex(PhysicsThreadMutex);
			continue;
		}
		// The function is copied to a local variable so we can unlock the mutex during its runtime
		const FixedUpdateFunction UpdateFunction = PhysicsThreadFunction;
		SDL_UnlockMutex(PhysicsThreadMutex);
		UpdateFunction(GetState());

		ulong timeEnd = SDL_GetTicks64();
		ulong timeElapsed = timeEnd - timeStart;
		while (timeElapsed < PHYSICS_TARGET_MS)
		{
			// SDL_Delay is inaccurate at low values (often 15 or lower)
			// and as we are targeting 60 TPS (16ms) we need more accuracy
			// so we are unfortunately forced to spin
			timeEnd = SDL_GetTicks64();
			timeElapsed = timeEnd - timeStart;
		}
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
