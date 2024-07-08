//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include "../defines.h"
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
/**
 * Extra verification, mainly for debugging (slower)
 * This will only work if the LunarG Vulkan SDK is installed on the device running the program.
 * @warning NOT FOR RELEASE BUILDS
 * @see https://docs.vulkan.org/guide/latest/validation_overview.html
 * @see https://vulkan.lunarg.com/doc/sdk/1.3.283.0/windows/khronos_validation_layer.html
 */
#define VALIDATION_ENABLE

typedef struct {
    unsigned int graphicsFamily;
    unsigned int presentFamily;
} QueueFamilyIndices;

typedef struct {
    unsigned int formatCount;
    VkSurfaceFormatKHR *formats;
    unsigned int presentModeCount;
    VkPresentModeKHR *presentMode;
    VkSurfaceCapabilitiesKHR capabilities;
} SwapChainSupportDetails;


void InitVulkan(SDL_Window *window);
void CleanupVulkan();

VkInstance GetVulkanInstance();
VkSurfaceKHR GetVulkanSurface();

#endif //GAME_VULKAN_H
