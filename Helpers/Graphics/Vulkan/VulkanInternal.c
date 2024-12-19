//
// Created by Noah on 11/23/2024.
//

#include "VulkanInternal.h"
#include <SDL_vulkan.h>
#include <string.h>
#include "VulkanHelpers.h"
#include "VulkanMemory.h"
#include "VulkanResources.h"
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"

bool CreateInstance()
{
    uint32_t extensionCount;
    if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, NULL) == SDL_FALSE)
    {
        VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!");
        return false;
    }
    const char *extensionNames[extensionCount];
    if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, extensionNames) == SDL_FALSE)
    {
        VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!");
        return false;
    }
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

#if defined(VK_ENABLE_VALIDATION_LAYER) || defined(VK_ENABLE_MESA_FPS_OVERLAY)
    uint32_t layerCount;
    VulkanTest(vkEnumerateInstanceLayerProperties(&layerCount, NULL), "Failed to enumerate Vulkan instance layers!");
    VkLayerProperties availableLayers[layerCount];
    VulkanTest(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers),
               "Failed to enumerate Vulkan instance layers!");
#endif
#if defined(VK_ENABLE_VALIDATION_LAYER) && defined(VK_ENABLE_MESA_FPS_OVERLAY)
    uint8_t found = 0;
    for (uint32_t i = 0; i < layerCount; i++)
    {
        if (!strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 28)) found |= 1;
        if (!strncmp(availableLayers[i].layerName, "VK_LAYER_MESA_overlay", 22)) found |= 2;
        if (found == 3) break;
    }
    if (found != 3)
    {
        if (found == 1)
        {
            FriendlyError("Missing Vulkan Mesa layers!",
                          "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay. "
                          "If you wish to disable the Mesa FPS overlay, that can be done by removing the definition for VK_ENABLE_MESA_FPS_OVERLAY in config.h");
        }
        FriendlyError("Missing Vulkan validation layers!",
                      "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\n"
                      "You can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package manager of your choice.\n"
                      "If you wish to disable the validation layer, that can be done by removing the definition for VK_ENABLE_VALIDATION_LAYER in config.h");
    }
    createInfo.enabledLayerCount = 2;
    createInfo.ppEnabledLayerNames = (const char *const[2]){"VK_LAYER_KHRONOS_validation", "VK_LAYER_MESA_overlay"};
#elifdef VK_ENABLE_VALIDATION_LAYER
    bool found = false;
    for (uint32_t i = 0; i < layerCount; i++)
    {
        if (!strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 27))
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        FriendlyError("Missing Vulkan validation layers!",
                      "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\n"
                      "You can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package manager of your choice.\n"
                      "If you wish to disable the validation layer, that can be done by removing the definition for VK_ENABLE_VALIDATION_LAYER in config.h");
    }
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#elifdef VK_ENABLE_MESA_FPS_OVERLAY
    bool found = false;
    for (uint32_t i = 0; i < layerCount; i++)
    {
        if (!strncmp(availableLayers[i].layerName, "VK_LAYER_MESA_overlay", 22))
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        FriendlyError("Missing Vulkan Mesa layers!",
                      "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay.\n"
                      "If you wish to disable the Mesa FPS overlay, that can be done by removing the definition for VK_ENABLE_MESA_FPS_OVERLAY in config.h");
    }
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_MESA_overlay"};
#endif

    VulkanTest(vkCreateInstance(&createInfo, NULL, &instance), "Failed to create Vulkan instance!")

    return true;
}

bool CreateSurface()
{
    if (SDL_Vulkan_CreateSurface(vk_window, instance, &surface) == SDL_FALSE)
    {
        VulkanLogError("Failed to create Vulkan window surface");
        return false;
    }

    return true;
}

bool PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    VulkanTest(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL), "Failed to enumerate physical devices!");
    if (deviceCount == 0)
    {
        VulkanLogError("Failed to find any GPUs with Vulkan support!");
        return false;
    }
    VkPhysicalDevice devices[deviceCount];
    VulkanTest(vkEnumeratePhysicalDevices(instance, &deviceCount, devices), "Failed to enumerate physical devices!");

    bool match = false;
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        queueFamilyIndices = (QueueFamilyIndices){-1, -1, -1, 0};
        const VkPhysicalDevice pDevice = devices[i];

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
        if (!deviceFeatures.geometryShader || !deviceFeatures.samplerAnisotropy) continue;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, NULL);
        VkQueueFamilyProperties families[familyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, families);
        for (uint32_t index = 0; index < familyCount; index++)
        {
            VkBool32 presentSupport = VK_FALSE;
            VulkanTest(vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, index, surface, &presentSupport),
                       "Failed to check Vulkan device for presentation support!");

            if (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueFamilyIndices.graphicsFamily = index;
                if (presentSupport) queueFamilyIndices.presentFamily = index;
            } else
            {
                if (presentSupport)
                {
                    queueFamilyIndices.uniquePresentFamily = index;
                }
            }

            if ((queueFamilyIndices.presentFamily == -1 && queueFamilyIndices.uniquePresentFamily == -1) ||
                queueFamilyIndices.graphicsFamily == -1)
            {
                continue;
            }

            break;
        }

        if (queueFamilyIndices.presentFamily == -1)
        {
            queueFamilyIndices.presentFamily = queueFamilyIndices.uniquePresentFamily;
        }

        if (queueFamilyIndices.graphicsFamily == queueFamilyIndices.presentFamily)
        {
            queueFamilyIndices.familyCount = 1;
        } else
        {
            queueFamilyIndices.familyCount = 2;
        }

        uint32_t extensionCount;
        VulkanTest(vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL),
                   "Failed to enumerate Vulkan device extensions!");
        if (extensionCount == 0) continue;
        VkExtensionProperties availableExtensions[extensionCount];
        VulkanTest(vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, availableExtensions),
                   "Failed to enumerate Vulkan device extensions!");
        for (uint32_t j = 0; j < extensionCount; j++)
        {
            if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
            {
                if (!QuerySwapChainSupport(pDevice))
                {
                    VulkanLogError("Failed to query Vulkan swap chain support!");
                    return false;
                }
                if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0) continue;

                VkPhysicalDeviceProperties deviceProperties;
                VkPhysicalDeviceMemoryProperties memoryProperties;

                vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
                vkGetPhysicalDeviceMemoryProperties(devices[i], &memoryProperties);

                if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    physicalDevice.device = devices[i];
                    physicalDevice.features = deviceFeatures;
                    physicalDevice.properties = deviceProperties;
                    physicalDevice.memoryProperties = memoryProperties;
                    return true;
                }
                physicalDevice.device = devices[i];
                physicalDevice.features = deviceFeatures;
                physicalDevice.properties = deviceProperties;
                physicalDevice.memoryProperties = memoryProperties;
                match = true;
                break;
            }
        }
    }

    if (!match) { VulkanLogError("Failed to find a suitable GPU for Vulkan!"); }

    return match;
}

bool CreateLogicalDevice()
{
    const float queuePriority = 1;

    VkDeviceQueueCreateInfo queueCreateInfo[queueFamilyIndices.familyCount];
    switch (queueFamilyIndices.familyCount)
    {
        case 2:
            queueCreateInfo[1] = (VkDeviceQueueCreateInfo){
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices.presentFamily,
                1,
                &queuePriority
            };
        case 1:
            queueCreateInfo[0] = (VkDeviceQueueCreateInfo){
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices.graphicsFamily,
                1,
                &queuePriority
            };
            break;
        default:
            VulkanLogError("Failed to create VkDeviceQueueCreateInfo due to invalid queueFamilyIndices!");
            return false;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {
        .logicOp = VK_TRUE,
        .samplerAnisotropy = VK_TRUE,
        .tessellationShader = VK_TRUE,
    };
    VkPhysicalDeviceVulkan12Features vulkan12Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = NULL,
        .runtimeDescriptorArray = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE
    };
    VkDeviceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        &vulkan12Features,
        0,
        queueFamilyIndices.familyCount,
        queueCreateInfo,
        0,
        NULL,
        1,
        (const char *const[1]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        &deviceFeatures
    };

#ifdef VK_ENABLE_VALIDATION_LAYER
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#endif

    VulkanTest(vkCreateDevice(physicalDevice.device, &createInfo, NULL, &device), "Failed to create Vulkan device!");

    vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);

    return true;
}

bool CreateSwapChain()
{
    if (minimized) return true;

    if (!QuerySwapChainSupport(physicalDevice.device))
    {
        VulkanLogError("Failed to query Vulkan swap chain support!");
        return false;
    }

    if (!swapChainSupport.capabilities.currentExtent.width || !swapChainSupport.capabilities.currentExtent.height)
    {
        // Window is minimized, so return to prevent creating a swap chain with dimensions of 0px by 0px
        // However, we do not want to fail or even log anything, since this is intended behavior
        minimized = true;
        return true;
    }

    VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR};
    for (uint32_t i = 0; i < swapChainSupport.formatCount; i++)
    {
        const VkSurfaceFormatKHR format = swapChainSupport.formats[i];
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM || surfaceFormat.format == VK_FORMAT_MAX_ENUM)
            {
                surfaceFormat = format;
            } else if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                surfaceFormat = format;
                break;
            }
        }
    }
    if (surfaceFormat.format == VK_FORMAT_MAX_ENUM || surfaceFormat.colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
    {
        VulkanLogError("Unable to find suitable Vulkan swap chain color format!");
        return false;
    }

    VkExtent2D extent = swapChainSupport.capabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
    {
        int32_t width;
        int32_t height;
        SDL_Vulkan_GetDrawableSize(vk_window, &width, &height);
        extent.width = clamp(width, swapChainSupport.capabilities.minImageExtent.width,
                             swapChainSupport.capabilities.maxImageExtent.width);
        extent.height = clamp(height, swapChainSupport.capabilities.minImageExtent.height,
                              swapChainSupport.capabilities.maxImageExtent.height);
    }
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
    switch (queueFamilyIndices.familyCount)
    {
        case 2:
            pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
            pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
            break;
        case 1:
            pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
            break;
        default:
            VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!");
            return false;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    if (!GetState()->options.vsync)
    {
        for (uint32_t i = 0; i < swapChainSupport.presentModeCount; i++)
        {
            const VkPresentModeKHR mode = swapChainSupport.presentMode[i];
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }
    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) presentMode = VK_PRESENT_MODE_FIFO_KHR;

    const VkSwapchainCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        queueFamilyIndices.familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices.familyCount,
        pQueueFamilyIndices,
        swapChainSupport.capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    VulkanTest(vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain), "Failed to create Vulkan swap chain!");

    VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL), "Failed to get Vulkan swapchain images!");
    swapChainImages = (VkImage *)malloc(sizeof(*swapChainImages) * imageCount);
    swapChainCount = imageCount;
    VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages),
               "Failed to get Vulkan swapchain images!");
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return true;
}

bool CreateImageViews()
{
    swapChainImageViews = (VkImageView *)malloc(sizeof(*swapChainImageViews) * swapChainCount);

    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        if (!CreateImageView(&swapChainImageViews[i], swapChainImages[i], swapChainImageFormat,
                             VK_IMAGE_ASPECT_COLOR_BIT, 1, "Failed to create Vulkan swap chain image view!"))
        {
            return false;
        }
    }

    return true;
}

bool CreateRenderPass()
{
    // TODO if stencil is not needed then allow for using VK_FORMAT_D16_UNORM
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice.device, VK_FORMAT_D24_UNORM_S8_UINT, &properties);
    if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        depthImageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    } else
    {
        vkGetPhysicalDeviceFormatProperties(physicalDevice.device, VK_FORMAT_D32_SFLOAT_S8_UINT, &properties);
        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
        } else
        {
            VulkanLogError("Unable to find suitable format for Vulkan depth image!");
            return false;
        }
    }

    switch (GetState()->options.msaa)
    {
        case MSAA_2X:
            msaaSamples = VK_SAMPLE_COUNT_2_BIT;
            break;
        case MSAA_4X:
            msaaSamples = VK_SAMPLE_COUNT_4_BIT;
            break;
        case MSAA_8X:
            msaaSamples = VK_SAMPLE_COUNT_8_BIT;
            break;
        case MSAA_NONE:
        default:
            msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            break;
    }
    if (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
          physicalDevice.properties.limits.framebufferDepthSampleCounts & msaaSamples))
    {
        ShowWarning("Invalid Settings", "Your GPU driver does not support the selected MSAA level!\n"
                    "A fallback has been set to avoid issues.");
        while (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
                 physicalDevice.properties.limits.framebufferDepthSampleCounts & msaaSamples))
        {
            msaaSamples >>= 1;
        }
    }

    const VkAttachmentReference colorAttachmentRef = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    const VkAttachmentReference depthAttachmentReference = {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    const VkSubpassDependency dependencies[2] = {
        {
            VK_SUBPASS_EXTERNAL,
            0,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            0
        },
        {
            0,
            1,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            0
        }
    };

    if (msaaSamples == VK_SAMPLE_COUNT_1_BIT)
    {
        const VkAttachmentDescription colorAttachment =
        {
            0,
            swapChainImageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        const VkAttachmentDescription depthAttachment = {
            0,
            depthImageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        const VkSubpassDescription wallSubpass = {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            NULL,
            1,
            &colorAttachmentRef,
            NULL,
            &depthAttachmentReference,
            0,
            NULL
        };
        const VkSubpassDescription uiSubpass = {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            NULL,
            1,
            &colorAttachmentRef,
            NULL,
            NULL,
            0,
            NULL
        };

        const VkRenderPassCreateInfo renderPassInfo = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            NULL,
            0,
            2,
            (VkAttachmentDescription[]){colorAttachment, depthAttachment},
            2,
            (VkSubpassDescription[]){wallSubpass, uiSubpass},
            2,
            dependencies
        };

        VulkanTest(vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass),
                   "Failed to create Vulkan render pass!");
    } else
    {
        const VkAttachmentDescription colorAttachment = {
            0,
            swapChainImageFormat,
            msaaSamples,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        const VkAttachmentDescription depthAttachment = {
            0,
            depthImageFormat,
            msaaSamples,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
        const VkAttachmentDescription colorResolveAttachment = {
            0,
            swapChainImageFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        const VkAttachmentReference colorAttachmentResolveRef = {
            2,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        const VkSubpassDescription wallSubpass = {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            NULL,
            1,
            &colorAttachmentRef,
            &colorAttachmentResolveRef,
            &depthAttachmentReference,
            0,
            NULL
        };
        const VkSubpassDescription uiSubpass = {
            0,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            0,
            NULL,
            1,
            &colorAttachmentRef,
            &colorAttachmentResolveRef,
            NULL,
            0,
            NULL
        };

        const VkRenderPassCreateInfo renderPassInfo = {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            NULL,
            0,
            3,
            (VkAttachmentDescription[]){colorAttachment, depthAttachment, colorResolveAttachment},
            2,
            (VkSubpassDescription[]){wallSubpass, uiSubpass},
            2,
            dependencies
        };

        VulkanTest(vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass),
                   "Failed to create Vulkan render pass!");
    }

    return true;
}

bool CreateDescriptorSetLayouts()
{
    const VkDescriptorSetLayoutBinding bindings[2] = {
        {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            1,
            VK_SHADER_STAGE_VERTEX_BIT,
            NULL
        },
        {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            TEXTURE_ASSET_COUNT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            NULL
        }
    };
    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        NULL,
        0,
        2,
        bindings
    };

    VulkanTest(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout),
               "Failed to create pipeline descriptor set layout!");

    return true;
}

bool CreateGraphicsPipelineCache()
{
    const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        NULL,
        0,
        0,
        NULL
    };
    VulkanTest(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL, &pipelineCache),
               "Failed to create graphics pipeline cache!");

    return true;
}

bool CreateGraphicsPipelines()
{
#pragma region shared
    const VkViewport viewport = {
        0,
        0,
        (float)swapChainExtent.width,
        (float)swapChainExtent.height,
        0,
        1
    };
    const VkRect2D scissor = {
        {
            0,
            0
        },
        swapChainExtent
    };
    const VkPipelineViewportStateCreateInfo viewportState = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        NULL,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0,
        0,
        0,
        1
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        NULL,
        0,
        msaaSamples,
        VK_FALSE,
        1,
        NULL,
        VK_FALSE,
        VK_FALSE
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencil = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        NULL,
        0,
        VK_TRUE,
        VK_TRUE,
        VK_COMPARE_OP_LESS,
        VK_FALSE,
        VK_FALSE,
        {},
        {},
        0,
        1
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_TRUE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    const VkPipelineColorBlendStateCreateInfo colorBlending = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        NULL,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &colorBlendAttachment,
        {0, 0, 0, 0}
    };

    const VkPipelineDynamicStateCreateInfo dynamicState = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        NULL,
        0,
        0,
        NULL
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &descriptorSetLayout,
        0,
        NULL
    };
    VulkanTest(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout),
               "Failed to create graphics pipeline layout!");
#pragma endregion shared

#pragma region walls
    const VkShaderModule wallVertShaderModule = CreateShaderModule((uint32_t *)DecompressAsset(gzvert_Vulkan_wall),
                                                                   AssetGetSize(gzvert_Vulkan_wall));
    const VkShaderModule wallFragShaderModule = CreateShaderModule((uint32_t *)DecompressAsset(gzfrag_Vulkan_wall),
                                                                   AssetGetSize(gzfrag_Vulkan_wall));
    if (!wallVertShaderModule || !wallFragShaderModule)
    {
        VulkanLogError("Failed to load wall shaders!");
        return false;
    }

    const VkPipelineShaderStageCreateInfo wallShaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            wallVertShaderModule,
            "main",
            NULL
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            wallFragShaderModule,
            "main",
            NULL
        }
    };

    const VkVertexInputBindingDescription wallBindingDescriptions[1] = {
        {
            0,
            sizeof(WallVertex),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };
    const VkVertexInputAttributeDescription wallVertexDescriptions[3] = {
        {
            0,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(WallVertex, x)
        },
        {
            1,
            0,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof(WallVertex, u)
        },
        {
            2,
            0,
            VK_FORMAT_R32_UINT,
            offsetof(WallVertex, textureIndex)
        }
    };
    const VkPipelineVertexInputStateCreateInfo wallVertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        wallBindingDescriptions,
        3,
        wallVertexDescriptions
    };

    const VkPipelineInputAssemblyStateCreateInfo wallInputAssembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        NULL,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    VkGraphicsPipelineCreateInfo wallsPipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        NULL,
        0,
        2,
        wallShaderStages,
        &wallVertexInputInfo,
        &wallInputAssembly,
        NULL,
        &viewportState,
        &rasterizer,
        &multisampling,
        &depthStencil,
        &colorBlending,
        &dynamicState,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };
#pragma endregion walls

#pragma region UI
    const VkShaderModule uiVertShaderModule = CreateShaderModule(
        (uint32_t *)DecompressAsset(gzvert_Vulkan_ui), AssetGetSize(gzvert_Vulkan_ui));
    const VkShaderModule uiFragShaderModule = CreateShaderModule(
        (uint32_t *)DecompressAsset(gzfrag_Vulkan_ui), AssetGetSize(gzfrag_Vulkan_ui));
    if (!uiVertShaderModule || !uiFragShaderModule)
    {
        VulkanLogError("Failed to load colored quad shaders!");
        return false;
    }

    const VkPipelineShaderStageCreateInfo uiShaderStages[2] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            uiVertShaderModule,
            "main",
            NULL
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            uiFragShaderModule,
            "main",
            NULL
        }
    };

    const VkVertexInputBindingDescription uiBindingDescription = {
        0,
        sizeof(UiVertex),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    const VkVertexInputAttributeDescription uiAttributeDescriptions[3] = {
        {
            0,
            0,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            offsetof(UiVertex, posXY_uvZW)
        },
        {
            1,
            0,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            offsetof(UiVertex, color)
        },
        {
            2,
            0,
            VK_FORMAT_R32_UINT,
            offsetof(UiVertex, textureIndex)
        },
    };
    const VkPipelineVertexInputStateCreateInfo uiVertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &uiBindingDescription,
        3,
        uiAttributeDescriptions
    };

    const VkPipelineInputAssemblyStateCreateInfo uiInputAssembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        NULL,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    VkGraphicsPipelineCreateInfo uiPipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        NULL,
        0,
        2,
        uiShaderStages,
        &uiVertexInputInfo,
        &uiInputAssembly,
        NULL,
        &viewportState,
        &rasterizer,
        &multisampling,
        NULL,
        &colorBlending,
        &dynamicState,
        pipelineLayout,
        renderPass,
        1,
        VK_NULL_HANDLE,
        -1
    };
#pragma endregion UI


    VkGraphicsPipelineCreateInfo pipelinesCreateInfo[2] = {
        wallsPipelineInfo,
        uiPipelineInfo,
    };
    VkPipeline pipelineList[2] = {0};

    VulkanTest(vkCreateGraphicsPipelines(device, pipelineCache, 2, pipelinesCreateInfo, NULL, pipelineList),
               "Failed to create graphics pipelines!");

    pipelines.walls = pipelineList[0];
    pipelines.ui = pipelineList[1];


    vkDestroyShaderModule(device, wallVertShaderModule, NULL);
    vkDestroyShaderModule(device, wallFragShaderModule, NULL);

    vkDestroyShaderModule(device, uiVertShaderModule, NULL);
    vkDestroyShaderModule(device, uiFragShaderModule, NULL);

    return true;
}

bool CreateCommandPools()
{
    const VkCommandPoolCreateInfo graphicsPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndices.graphicsFamily
    };

    VulkanTest(vkCreateCommandPool(device, &graphicsPoolInfo, NULL, &graphicsCommandPool),
               "Failed to create Vulkan graphics command pool!");

    return true;
}

bool CreateColorImage()
{
    if (!CreateImage(&colorImage, &colorImageMemory, swapChainImageFormat,
                     (VkExtent3D){swapChainExtent.width, swapChainExtent.height, 1}, 1,
                     msaaSamples, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     "color"))
    {
        return false;
    }

    if (!CreateImageView(&colorImageView, colorImage, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,
                         1, "Failed to create Vulkan color image view!"))
    {
        return false;
    }

    return true;
}

bool CreateDepthImage()
{
    if (!CreateImage(&depthImage, &depthImageMemory, depthImageFormat,
                     (VkExtent3D){swapChainExtent.width, swapChainExtent.height, 1}, 1,
                     msaaSamples, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                     "depth test"))
    {
        return false;
    }

    if (!CreateImageView(&depthImageView, depthImage, depthImageFormat,
                         VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                         1, "Failed to create Vulkan depth image view!"))
    {
        return false;
    }

    const VkCommandBuffer commandBuffer;
    if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool)) return false;

    const VkImageMemoryBarrier transferBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        NULL,
        0,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        depthImage,
        {
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            0,
            1,
            0,
            1
        }
    };

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                         0, 0, NULL, 0, NULL, 1, &transferBarrier);

    if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue)) return false;

    return true;
}

bool CreateFramebuffers()
{
    swapChainFramebuffers = (VkFramebuffer *)malloc(sizeof(*swapChainFramebuffers) * swapChainCount);

    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        VkFramebufferCreateInfo framebufferInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            renderPass,
            msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3,
            msaaSamples == VK_SAMPLE_COUNT_1_BIT
                ? (VkImageView[2]){swapChainImageViews[i], depthImageView}
                : (VkImageView[3]){colorImageView, depthImageView, swapChainImageViews[i]},
            swapChainExtent.width,
            swapChainExtent.height,
            1
        };

        VulkanTest(vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]),
                   "Failed to create Vulkan framebuffer!");
    }

    return true;
}

bool LoadTextures()
{
    VkDeviceSize memorySize = 0;
    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        const uint8_t *decompressed = DecompressAsset(texture_assets[textureIndex]);

        const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice.device, format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            VulkanLogError("Vulkan texture image format does not support linear blitting!");
            return false;
        }

        const VkExtent3D extent = {ReadUintA(decompressed, IMAGE_WIDTH_OFFSET), ReadUintA(decompressed, IMAGE_HEIGHT_OFFSET), 1};
        textures[textureIndex].mipmapLevels = GetState()->options.mipmaps
                                                  ? (uint8_t)log2(max(extent.width, extent.height)) + 1
                                                  : 1;
        if (!CreateImage(&textures[textureIndex].image, NULL, format, extent, textures[textureIndex].mipmapLevels,
                         VK_SAMPLE_COUNT_1_BIT,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                         "texture"))
        {
            return false;
        }

        vkGetImageMemoryRequirements(device, textures[textureIndex].image,
                                     &textures[textureIndex].allocationInfo.memoryRequirements);

        const VkDeviceSize alignment = textures[textureIndex].allocationInfo.memoryRequirements.alignment;
        const double alignedSize = (double)(memorySize + textures[textureIndex].allocationInfo.memoryRequirements.size);
        textures[textureIndex].allocationInfo.offset = alignment * (VkDeviceSize)ceil(alignedSize / (double)alignment);

        memorySize = textures[textureIndex].allocationInfo.offset +
                     textures[textureIndex].allocationInfo.memoryRequirements.size;

        texturesAssetIDMap[ReadUintA(decompressed, IMAGE_ID_OFFSET)] = textureIndex;
    }

    for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
    {
        if (textures[i].allocationInfo.memoryRequirements.memoryTypeBits & 1 << i &&
            (physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            const VkMemoryAllocateInfo allocateInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memorySize,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocateInfo, NULL, &textureMemory),
                       "Failed to allocate Vulkan texture memory!");
            break;
        }
    }

    VkBuffer stagingBuffer;
    MemoryInfo memoryInfo = {
        0,
        NULL,
        0,
        0,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    MemoryAllocationInfo allocationInfo = {
        0,
        &memoryInfo,
        {0}
    };
    if (!CreateBuffer(&stagingBuffer, memorySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, &allocationInfo))
    {
        return false;
    }

    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        VulkanTest(
            vkBindImageMemory(device, textures[textureIndex].image, textureMemory,
                textures[textureIndex].allocationInfo.offset), "Failed to bind Vulkan texture memory!");

        const uint8_t *decompressed = DecompressAsset(texture_assets[textureIndex]);
        uint32_t width = ReadUintA(decompressed, IMAGE_WIDTH_OFFSET);
        uint32_t height = ReadUintA(decompressed, IMAGE_HEIGHT_OFFSET);
        void *data;

        VulkanTest(vkMapMemory(device, memoryInfo.memory, textures[textureIndex].allocationInfo.offset,
                       textures[textureIndex].allocationInfo.memoryRequirements.size, 0, &data),
                   "Failed to map Vulkan texture staging buffer memory!");

        memcpy(data, decompressed + sizeof(uint32_t) * 4, ReadUintA(decompressed, IMAGE_SIZE_OFFSET) * 4);
        vkUnmapMemory(device, memoryInfo.memory);

        const VkCommandBuffer commandBuffer;
        if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool)) return false;

        const VkImageMemoryBarrier transferBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            NULL,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            textures[textureIndex].image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                textures[textureIndex].mipmapLevels,
                0,
                1
            }
        };

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             NULL, 0, NULL, 1, &transferBarrier);

        if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue)) return false;

        if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool)) return false;

        const VkBufferImageCopy bufferCopyInfo = {
            textures[textureIndex].allocationInfo.offset,
            0,
            0,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                0,
                1
            },
            {0, 0, 0},
            {
                width,
                height,
                1
            }
        };

        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, textures[textureIndex].image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyInfo);

        if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue)) return false;

        if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool)) return false;

        for (uint8_t level = 0; level < textures[textureIndex].mipmapLevels - 1; level++)
        {
            const VkImageMemoryBarrier blitBarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                NULL,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                textures[textureIndex].image,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    level,
                    1,
                    0,
                    1
                }
            };

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                 NULL, 0, NULL, 1, &blitBarrier);

            VkImageBlit blit = {
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    level,
                    0,
                    1
                },
                {
                    {0, 0, 0},
                    {(int32_t)width, (int32_t)height, 1}
                },
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    level + 1,
                    0,
                    1
                },
                {
                    {0, 0, 0},
                    {width > 1 ? (int32_t)width / 2 : 1, height > 1 ? (int32_t)height / 2 : 1, 1}
                }
            };

            vkCmdBlitImage(commandBuffer, textures[textureIndex].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           textures[textureIndex].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                           VK_FILTER_LINEAR);

            const VkImageMemoryBarrier mipmapBarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                NULL,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                textures[textureIndex].image,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    level,
                    1,
                    0,
                    1
                }
            };

            // TODO Best practices validation doesn't like this
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, NULL, 0, NULL, 1, &mipmapBarrier);

            if (width > 1) width /= 2;
            if (height > 1) height /= 2;
        }

        const VkImageMemoryBarrier mipmapBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            NULL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            textures[textureIndex].image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                textures[textureIndex].mipmapLevels - 1,
                1,
                0,
                1
            }
        };

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, NULL, 0, NULL, 1, &mipmapBarrier);

        if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue)) return false;
    }

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, memoryInfo.memory, NULL);

    return true;
}

bool CreateTexturesImageView()
{
    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        if (!CreateImageView(&texturesImageView[textureIndex], textures[textureIndex].image, VK_FORMAT_R8G8B8A8_UNORM,
                             VK_IMAGE_ASPECT_COLOR_BIT, textures[textureIndex].mipmapLevels,
                             "Failed to create Vulkan texture image view!"))
        {
            return false;
        }
    }

    return true;
}

bool CreateTextureSampler()
{
    const VkSamplerCreateInfo linearRepeatSamplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        NULL,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        -1.5f,
        VK_FALSE,
        1,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0,
        VK_LOD_CLAMP_NONE,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        VK_FALSE
    };
    const VkSamplerCreateInfo nearestRepeatSamplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        NULL,
        0,
        VK_FILTER_NEAREST,
        VK_FILTER_NEAREST,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        -1.5f,
        VK_FALSE,
        1,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0,
        VK_LOD_CLAMP_NONE,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        VK_FALSE
    };
    const VkSamplerCreateInfo linearNoRepeatSamplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        NULL,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        -1.5f,
        VK_FALSE,
        1,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0,
        VK_LOD_CLAMP_NONE,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        VK_FALSE
    };
    const VkSamplerCreateInfo nearestNoRepeatSamplerCreateInfo = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        NULL,
        0,
        VK_FILTER_NEAREST,
        VK_FILTER_NEAREST,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        -1.5f,
        VK_FALSE,
        1,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0,
        VK_LOD_CLAMP_NONE,
        VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        VK_FALSE
    };

    VulkanTest(vkCreateSampler(device, &linearRepeatSamplerCreateInfo, NULL, &textureSamplers.linearRepeat),
               "Failed to create linear repeating texture sampler!");
    VulkanTest(vkCreateSampler(device, &nearestRepeatSamplerCreateInfo, NULL, &textureSamplers.nearestRepeat),
               "Failed to create nearest repeating texture sampler!");
    VulkanTest(vkCreateSampler(device, &linearNoRepeatSamplerCreateInfo, NULL, &textureSamplers.linearNoRepeat),
               "Failed to create linear non-repeating texture sampler!");
    VulkanTest(vkCreateSampler(device, &nearestNoRepeatSamplerCreateInfo, NULL, &textureSamplers.nearestNoRepeat),
               "Failed to create nearest non-repeating texture sampler!");

    return true;
}

bool CreateBuffers()
{
    buffers.walls.maxWallCount = MAX_WALLS_INIT;
    CreateLocalBuffer();
    SetLocalBufferAliasingInfo();

    buffers.ui.maxQuads = MAX_UI_QUADS_INIT;
    CreateSharedBuffer();
    SetSharedBufferAliasingInfo();

    return true;
}

bool AllocateMemory()
{
    if (!CreateLocalMemory()) return false;
    if (!CreateSharedMemory()) return false;

    return true;
}

bool CreateDescriptorPool()
{
    const VkDescriptorPoolSize poolSizes[3] = {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            MAX_FRAMES_IN_FLIGHT * MAX_FRAMES_IN_FLIGHT
        },
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            TEXTURE_ASSET_COUNT * MAX_FRAMES_IN_FLIGHT
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            MAX_FRAMES_IN_FLIGHT
        }
    };
    const VkDescriptorPoolCreateInfo poolCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0,
        MAX_FRAMES_IN_FLIGHT,
        3,
        poolSizes
    };

    VulkanTest(vkCreateDescriptorPool(device, &poolCreateInfo, NULL, &descriptorPool),
               "Failed to create descriptor pool!");

    return true;
}

bool CreateDescriptorSets()
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = descriptorSetLayout;
    }

    const VkDescriptorSetAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        NULL,
        descriptorPool,
        MAX_FRAMES_IN_FLIGHT,
        layouts
    };

    VulkanTest(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets),
               "Failed to allocate Vulkan descriptor sets!");

    UpdateDescriptorSets();

    return true;
}

bool CreateCommandBuffers()
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        graphicsCommandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        MAX_FRAMES_IN_FLIGHT
    };

    VulkanTest(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers),
               "Failed to allocate Vulkan command buffers!");

    return true;
}

bool CreateSyncObjects()
{
    const VkSemaphoreCreateInfo semaphoreInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        NULL,
        0
    };

    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        NULL,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]),
                   "Failed to create Vulkan semaphores!");

        VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]),
                   "Failed to create Vulkan semaphores!");

        VulkanTest(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]), "Failed to create Vulkan semaphores!");
    }

    return true;
}

bool RecreateSwapChain()
{
    VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for Vulkan device to become idle!");

    CleanupSwapChain();
    CleanupColorImage();
    CleanupDepthImage();
    CleanupPipeline();
    CleanupSyncObjects();

    return CreateSwapChain() && CreateImageViews() && CreateGraphicsPipelines() && CreateColorImage() &&
           CreateDepthImage() && CreateFramebuffers() && CreateSyncObjects();
}
