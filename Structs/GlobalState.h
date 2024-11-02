//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_GLOBALSTATE_H
#define GAME_GLOBALSTATE_H

#include "../defines.h"

#define MUSIC_COUNT 1
extern const byte *music[MUSIC_COUNT];

/**
 * Initialize the global state
 * @warning This function should only be called once
 */
void InitState();

/**
 * Update the volume from the options
 * @note This function should be called whenever the options are changed
 */
void UpdateVolume();

/**
 * Show a text box
 * @param tb Text box to show
 */
void ShowTextBox(TextBox tb);

/**
 * Get the global state
 * @return the global state
 */
GlobalState *GetState();

/**
 * Damage the player
 * @param damage Damage to deal
 */
void TakeDamage(int damage);

/**
 * Heal the player
 * @param amount Amount to heal
 */
void Heal(int amount);

/**
 * Add ammo to the player
 * @param amount Amount to add
 */
void AddAmmo(int amount);

/**
 * Use ammo
 * @param amount amount to use
 */
void UseAmmo(int amount);

/**
 * Set game state update callback
 * @param UpdateGame update callback
 * @param FixedUpdateGame fixed-FPS update callback
 * @param currentState used for checking the state
 */
void SetUpdateCallback(void (*UpdateGame)(GlobalState* State), uint (*FixedUpdateGame)(uint interval, GlobalState* State), CurrentState currentState);
/**
 * Set game state render callback
 * @param RenderGame render callback
 */
void SetRenderCallback(void (*RenderGame)(GlobalState* State));

/**
 * Change the current level
 * @param l Level to change to
 */
void ChangeLevel(Level *l);

/**
 * Change the bgm
 * @param asset Asset to change to
 */
void ChangeMusic(const byte *asset);

/**
 * Stop the bgm
 */
void StopMusic();

/**
 * Attempt to play a sound effect
 * @param asset Sound effect to play
 * @warning If there are no free channels, the sound effect will not play, and you will not be notified
 */
void PlaySoundEffect(const byte *asset);

/**
 * Destroy the global state
 */
void DestroyGlobalState();

/**
 * Change the level by ID
 * @param id Level ID to change to
 */
void ChangeLevelByID(int id);

#endif //GAME_GLOBALSTATE_H
