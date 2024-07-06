//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include "../defines.h"

/// Extra verification, mainly for debugging (slower)
/// @see https://docs.vulkan.org/guide/latest/validation_overview.html
/// @see https://vulkan.lunarg.com/doc/sdk/1.3.283.0/windows/khronos_validation_layer.html
//#define VALIDATION_ENABLE

void initVulkan(SDL_Window *window);

VkInstance* GetVulkanInstance();
VkSurfaceKHR* GetVulkanSurface();

#endif //GAME_VULKAN_H
