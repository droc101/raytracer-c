//
// Created by droc101 on 11/10/2024.
//

#ifndef PLATFORMHELPERS_H
#define PLATFORMHELPERS_H

#include <SDL_video.h>

void DwmDarkMode(SDL_Window *window);

#ifdef WIN32
void *reallocarray(void *ptr, size_t nmemb, size_t size);
#endif

#endif //PLATFORMHELPERS_H
