//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"

VkInstance instance;
VkSurfaceKHR surface;
const char* validationLayer = "VK_LAYER_KHRONOS_validation";

void InitVulkan(SDL_Window *window) {
    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
    const char *extensionNames[extensionCount];
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames);
    VkApplicationInfo applicationInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "game",
            VK_MAKE_VERSION(1, 0, 0),
            "game",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_3
    };
    VkInstanceCreateInfo instanceCreateInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &applicationInfo,
            0,
            NULL,
            extensionCount,
            extensionNames
    };
#ifdef VALIDATION_ENABLE
    unsigned int layerCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &layerCount, NULL);
    const char *layerNames[layerCount];
    SDL_Vulkan_GetInstanceExtensions(window, &layerCount, layerNames);
    instanceCreateInfo.enabledLayerCount = layerCount;
    instanceCreateInfo.ppEnabledLayerNames = layerNames;
#endif

    vkCreateInstance(&instanceCreateInfo, NULL, &instance);
    SDL_Vulkan_CreateSurface(window, instance, &surface);
}

VkInstance* GetVulkanInstance() {
    return &instance;
}

VkSurfaceKHR* GetVulkanSurface() {
    return &surface;
}
