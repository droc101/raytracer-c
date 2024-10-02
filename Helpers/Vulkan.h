//
// Created by Noah on 7/5/2024.
//

#ifndef GAME_VULKAN_H
#define GAME_VULKAN_H

#include "../defines.h"
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "../cglm-0.9.4/include/cglm/cglm.h"

#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#ifndef NDEBUG
/**
 * Extra verification, mainly for debugging (slower)
 * This will only work if the LunarG Vulkan SDK is installed on the device running the program.
 * @warning NOT FOR RELEASE BUILDS
 * @see https://docs.vulkan.org/guide/latest/validation_overview.html
 * @see https://vulkan.lunarg.com/doc/sdk/1.3.283.0/windows/khronos_validation_layer.html
 */
#define VALIDATION_ENABLE
#endif
#define MAX_FRAMES_IN_FLIGHT 2

#define clamp(val, min, max)(val < min ? min : val > max ? max : val)

void InitVulkan(SDL_Window *window);
void DrawFrame();
void CleanupVulkan();

VkInstance GetVulkanInstance();
VkSurfaceKHR GetVulkanSurface();
VkDevice GetVulkanDevice();

#endif //GAME_VULKAN_H
