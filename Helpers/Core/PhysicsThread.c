//
// Created by droc101 on 12/12/24.
//

#include "PhysicsThread.h"

#include <SDL_thread.h>

#include "Logging.h"
#include "MathEx.h"
#include "../../defines.h"
#include "../../Structs/GlobalState.h"

SDL_Thread* PhysicsThread;
SDL_mutex* PhysicsThreadMutex;

void (*PhysicsThreadFunction)(GlobalState *state);
bool PhysicsThreadPostQuit = false;

void PhysicsNopFunction(GlobalState *state)
{
    SDL_Delay(PHYSICS_TARGET_MS - 2); // let the computer rest
}

int PhysicsThreadMain(void*)
{
    while (true)
    {
        uint timeStart = SDL_GetTicks();
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

        uint timeEnd = SDL_GetTicks();
        uint timeElapsed = timeEnd - timeStart;
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
    if (function == NULL)
    {
        function = PhysicsNopFunction;
    }
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
