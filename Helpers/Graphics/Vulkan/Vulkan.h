//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include <vulkan/vulkan.h>
#include "../../../defines.h"

/**
 * This function is used to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
bool VK_Init(SDL_Window *window);

VkResult VK_DrawFrame();

/// A function used to destroy the Vulkan objects when they are no longer needed.
bool VK_Cleanup();

void VK_Minimize();

void VK_Restore();

VkSampleCountFlags VK_MaxSampleCount();

#endif //GAME_VULKAN_H
