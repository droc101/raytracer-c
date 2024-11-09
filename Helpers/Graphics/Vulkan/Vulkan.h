//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include "../../../defines.h"

bool VK_Init(SDL_Window *window);
void VK_DrawFrame();
void VK_Cleanup();
void VK_Minimize();
void VK_Restore();

#endif //GAME_VULKAN_H
