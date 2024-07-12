//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LEVEL_H
#define GAME_LEVEL_H

#include "../defines.h"
#include "Vector2.h"
#include "Wall.h"
#include "../Helpers/List.h"
#include "SDL.h"

/**
 * Create a default empty level
 * @return Blank level
 */
Level * CreateLevel();

/**
 * Destroy a level and everything in it
 * @param l Level to destroy
 */
void DestroyLevel(Level *l);

/**
 * Bake a static wall array from the level
 * @param l Level to bake
 */
void BakeWallArray(Level *l);

/**
 * Bake a static actor array from the level
 * @param l Level to bake
 */
void BakeActorArray(Level *l);

/**
 * Add an actor to the level
 * @param actor Actor to add
 * @note This is intended to be used during gameplay, not level loading
 */
void AddActor(Actor* actor);

/**
 * Remove an actor from the level
 * @param actor Actor to remove
 * @note This is intended to be used during gameplay, not level loading
 */
void RemoveActor(Actor* actor);

/**
 * Render the level
 * @param camPos Camera position
 * @param camRot Camera rotation
 * @param fakeHeight Camera fake height
 */
void RenderLevel(Vector2 camPos, double camRot, double fakeHeight);

#endif //GAME_LEVEL_H
