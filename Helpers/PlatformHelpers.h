//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL_video.h>

/**
 * Attempt to enable dark mode on Windows 10
 * @param window The window to enable dark mode on
 */
void DwmDarkMode(SDL_Window *window);

#endif //PLATFORMHELPERS_H
