//
// Created by droc101 on 12/12/24.
//

#include "PhysicsThread.h"
#include <SDL_thread.h>
#include "../../defines.h"
#include "../../Structs/GlobalState.h"

SDL_Thread* PhysicsThread;
SDL_mutex* PhysicsThreadMutex;

/**
 * The function to run in the physics thread
 * @warning Only touch this when you have a lock on the mutex
 */
void (*PhysicsThreadFunction)(GlobalState *state);

/**
 * Whether to quit the physics thread on the next iteration
 * @warning Only touch this when you have a lock on the mutex
 */
bool PhysicsThreadPostQuit = false;

/**
 * The main function for the physics thread
 * @return 0
 */
int PhysicsThreadMain(void*)
{
    while (true)
    {
        const ulong timeStart = SDL_GetTicks();
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
        void (*function)(GlobalState *state) = PhysicsThreadFunction;
        SDL_UnlockMutex(PhysicsThreadMutex);
        function(GetState());

        const ulong timeEnd = SDL_GetTicks();
        const ulong timeElapsed = timeEnd - timeStart;
        if (timeElapsed < PHYSICS_TARGET_MS)
        {
            SDL_Delay(PHYSICS_TARGET_MS - timeElapsed);
        }
    }
}

void PhysicsThreadInit()
{
    PhysicsThreadFunction = NULL;
    PhysicsThreadPostQuit = false;
    PhysicsThreadMutex = SDL_CreateMutex();
    PhysicsThread = SDL_CreateThread(PhysicsThreadMain, "GamePhysics", NULL);
}

void PhysicsThreadSetFunction(void (*function)(GlobalState *state))
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
