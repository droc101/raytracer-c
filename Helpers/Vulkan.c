//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "Error.h"

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

/// A Vulkan instance is the connection between the game and the driver, through Vulkan.
/// The creation of it requires configuring Vulkan for the app, allowing for better driver performance.
VkInstance instance;
/// The interface between Vulkan and SDL, allowing Vulkan to actually draw to the window.
VkSurfaceKHR surface;
/// This stores the GPU.
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
QueueFamilyIndices queueFamilyIndices;
SwapChainSupportDetails swapChainSupport;
/// This is used for interfacing with the physical device.
VkDevice device;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue graphicsQueue;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue presentQueue;
/// Allows Vulkan to give a surface the rendered image.
VkSwapchainKHR swapChain;
VkImage *swapChainImages;
unsigned int swapChainImageCount;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;

/**
 * This function finds the indices of the queue families in QueueFamilyIndices.
 * @param pDevice The device to search within
 * @return A @c QueueFamilyIndices struct with values set either to the index of the queue family, or to `(unsigned int) -1` if the queue was not found
 * @see https://docs.vulkan.org/guide/latest/queues.html#_queue_family
 */
static inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pDevice) {
    QueueFamilyIndices indices = {-1, -1};
    unsigned int familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, NULL);
    VkQueueFamilyProperties families[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, families);
    for (unsigned int i = 0; i < familyCount; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.graphicsFamily != 0 && indices.presentFamily != 0) break;
    }
    return indices;
}

/**
 * Provides information about the physical device's support for swapchain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
static inline SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice pDevice) {
    SwapChainSupportDetails details = {0, NULL, 0, NULL};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, NULL);
    if (details.formatCount != 0) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR *) * details.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, details.formats);
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.presentMode = malloc(sizeof(VkPresentModeKHR *) * details.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, details.presentMode);
    }
    return details;
}

/**
 * This function will create the Vulkan instance, set up for SDL.
 * @param window The window to initialize Vulkan for.
 * @see instance
 */
static inline void CreateInstance(SDL_Window *window) {
    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
    const char *extensionNames[extensionCount];
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames);
    VkApplicationInfo applicationInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            GAME_TITLE,
            VULKAN_VERSION,
            GAME_TITLE,
            VULKAN_VERSION,
            VK_API_VERSION_1_3
    };
    VkInstanceCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &applicationInfo,
            0,
            NULL,
            extensionCount,
            extensionNames
    };
#if !defined(NDEBUG) && defined(VALIDATION_ENABLE)
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]) {"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        Error("Failed to create Vulkan instance!");
    }
}

/**
 * Creates the Vulkan surface
 * @param window The window the surface should be linked to
 * @see surface
 */
static inline void CreateSurface(SDL_Window *window) {
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE) {
        Error("Failed to create Vulkan window surface");
    }
}

/**
 * This function selects the GPU that will be used to render the game.
 * Assuming I did it right, it will pick the best GPU available.
 * @see FindQueueFamilies
 */
void PickPhysicalDevice() {
    unsigned int deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        Error("Failed to find any GPUs with Vulkan support!");
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    bool match = false;
    for (unsigned int i = 0; i < deviceCount; i++) {
        VkPhysicalDevice pDevice = devices[i];
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
        if (!deviceFeatures.geometryShader) continue;
        queueFamilyIndices = FindQueueFamilies(pDevice);
        if (queueFamilyIndices.graphicsFamily == -1) continue;
        if (queueFamilyIndices.presentFamily == -1) continue;
        unsigned int extensionCount;
        vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL);
        if (extensionCount == 0) continue;
        VkExtensionProperties availableExtensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, availableExtensions);
        bool extensionFound = false;
        for (unsigned int j = 0; j < extensionCount; j++) {
            if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) continue;
        swapChainSupport = QuerySwapChainSupport(pDevice);
        if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0) continue;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = devices[i];
            return;
        } else {
            physicalDevice = devices[i];
            match = true;
        }
    }
    if (!match) Error("Could not find a suitable GPU!");
}

/**
 * Creates the logical device that is used to interface with the physical device.
 * @see FindQueueFamilies
 */
void CreateLogicalDevice() {
    float queuePriority = 1;
    unsigned int queueCount = 1;
    VkDeviceQueueCreateInfo queueCreateInfo[2];
    queueCreateInfo[0] = (VkDeviceQueueCreateInfo) {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,
            0,
            queueFamilyIndices.graphicsFamily,
            1,
            &queuePriority
    };
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        queueCreateInfo[queueCount++] = (VkDeviceQueueCreateInfo) {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices.presentFamily,
                1,
                &queuePriority
        };
    }
    VkPhysicalDeviceFeatures deviceFeatures = {VK_FALSE};
    VkDeviceCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            NULL,
            0,
            queueCount,
            queueCreateInfo,
            0,
            NULL,
            1,
            (const char *const[1]) {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
            &deviceFeatures
    };
#if !defined(NDEBUG) && defined(VALIDATION_ENABLE)
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]) {"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
        Error("Failed to create Vulkan device!");
    }
    vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"

static inline VkSurfaceFormatKHR GetSwapSurfaceFormat(SwapChainSupportDetails details) {
    for (unsigned int i = 0; i < details.formatCount; i++) {
        VkSurfaceFormatKHR format = details.formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return details.formats[0];
}

static inline VkPresentModeKHR GetSwapPresentMode(SwapChainSupportDetails details) {
    for (unsigned int i = 0; i < details.presentModeCount; i++) {
        VkPresentModeKHR presentMode = details.presentMode[i];
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

#pragma clang diagnostic pop

static inline void CreateSwapChain() {
    VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat(swapChainSupport);
    VkPresentModeKHR presentMode = GetSwapPresentMode(swapChainSupport);

    unsigned int imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            NULL,
            0,
            surface,
            imageCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            swapChainSupport.capabilities.currentExtent,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            swapChainSupport.capabilities.currentTransform,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            presentMode,
            VK_TRUE,
            VK_NULL_HANDLE
    };
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = (const unsigned int[2]) {queueFamilyIndices.graphicsFamily,
                                                                  queueFamilyIndices.presentFamily};
    }
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) {
        Error("Failed to create Vulkan swap chain!");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    swapChainImages = malloc(sizeof(VkImage *) * imageCount);
    swapChainImageCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = swapChainSupport.capabilities.currentExtent;
}

static inline void CreateImageViews() {
    swapChainImageViews = malloc(sizeof(VkImageView *) * swapChainImageCount);
    for (unsigned int i = 0; i < swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                NULL,
                0,
                swapChainImages[i],
                VK_IMAGE_VIEW_TYPE_2D,
                swapChainImageFormat,
                {VK_COMPONENT_SWIZZLE_IDENTITY},
                {
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        0,
                        1,
                        0,
                        1
                }
        };
        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
            Error("Failed to create Vulkan image views!");
        }
    }
}

/**
 * This function is used to to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
void InitVulkan(SDL_Window *window) {
    CreateInstance(window);
    CreateSurface(window);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
}

/// A function used to destroy the Vulkan objects when they are no longer needed.
void CleanupVulkan() {
    for (unsigned int i = 0; i < swapChainImageCount; i++) {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
}

/// Gets a pointer to the Vulkan instance
VkInstance *GetVulkanInstance() {
    return &instance;
}

/// Gets a pointer to the Vulkan surface
VkSurfaceKHR *GetVulkanSurface() {
    return &surface;
}
