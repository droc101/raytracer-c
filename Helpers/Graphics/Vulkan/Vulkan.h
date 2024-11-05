//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include "../../../defines.h"

#ifndef NDEBUG
/**
 * Extra verification, mainly for debugging (slower)
 * This will only work if the LunarG Vulkan SDK is installed on the device running the program.
 * @warning NOT FOR RELEASE BUILDS
 * @see https://docs.vulkan.org/guide/latest/validation_overview.html
 * @see https://vulkan.lunarg.com/doc/sdk/1.3.283.0/windows/khronos_validation_layer.html
 */
#define VK_VALIDATION_ENABLE
#endif

bool VK_Init(SDL_Window *window);
void VK_DrawFrame();
void VK_Cleanup();
void VK_Minimize();
void VK_Restore();

#endif //GAME_VULKAN_H
