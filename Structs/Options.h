//
// Created by droc101 on 10/27/24.
//

#ifndef GAME_OPTIONS_H
#define GAME_OPTIONS_H

#include "../defines.h"

/**
 * Read options from disk, or load defaults if no options file is found or is invalid
 * @param options The options to load into
 */
void LoadOptions(Options *options);

/**
 * Save options to disk
 * @param options The options to save
 */
void SaveOptions(Options *options);

#endif //GAME_OPTIONS_H
