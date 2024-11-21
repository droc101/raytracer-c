//
// Created by Noah on 7/5/2024.
//

#include "VulkanInternal.h"

#pragma region internalFunctions
#pragma region helperFunctions
static bool QuerySwapChainSupport(const VkPhysicalDevice pDevice)
{
    swapChainSupport = malloc(sizeof(*swapChainSupport));
    SwapChainSupportDetails details = {0, NULL, 0, NULL, {}};

    VulkanTest(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities),
               "Failed to query Vulkan surface capabilities!");

    VulkanTest(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, NULL),
               "Failed to query Vulkan surface color formats!");
    if (details.formatCount != 0)
    {
        details.formats = malloc(sizeof(*details.formats) * details.formatCount);
        VulkanTest(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, details.formats),
                   "Failed to query Vulkan surface color formats!");
    }

    VulkanTest(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, NULL),
               "Failed to query Vulkan surface presentation modes!");
    if (details.presentModeCount != 0)
    {
        details.presentMode = malloc(sizeof(*details.presentMode) * details.presentModeCount);
        VulkanTest(
            vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, details.presentMode),
            "Failed to query Vulkan surface presentation modes!");
    }
    *swapChainSupport = details;

    return true;
}

static bool CreateImageView(VkImageView *imageView, const VkImage image, const VkFormat format,
                            const VkImageAspectFlagBits aspectMask, const uint8_t mipmapLevels,
                            const char *errorMessage)
{
    const VkImageViewCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        NULL,
        0,
        image,
        VK_IMAGE_VIEW_TYPE_2D,
        format,
        {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        {
            aspectMask,
            0,
            mipmapLevels,
            0,
            1
        }
    };

    VulkanTest(vkCreateImageView(device, &createInfo, NULL, imageView), errorMessage);

    return true;
}

static VkShaderModule CreateShaderModule(const uint32_t *code, const size_t size)
{
    VkShaderModule shaderModule;

    const VkShaderModuleCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL,
        0,
        size - 16,
        code
    };

    VulkanTestWithReturn(vkCreateShaderModule(device, &createInfo, NULL, &shaderModule), NULL,
                         "Failed to create shader module!");

    return shaderModule;
}

static bool CreateImage(VkImage *image, VkDeviceMemory *imageMemory, const VkFormat format, const VkExtent3D extent,
                        const uint8_t mipmapLevels, const VkSampleCountFlags samples,
                        const VkImageUsageFlags usageFlags, const char *imageType)
{
    uint32_t pQueueFamilyIndices[queueFamilyIndices->familyCount];
    switch (queueFamilyIndices->familyCount)
    {
        case 3:
            pQueueFamilyIndices[2] = queueFamilyIndices->graphicsFamily;
        case 2:
            if (queueFamilyIndices->presentFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->transferFamily;
            }
            else if (queueFamilyIndices->transferFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->presentFamily;
            }
            else
            {
                VulkanLogError("Failed to create VkImageCreateInfoKHR due to invalid queueFamilyIndices!");
                return false;
            }
        case 1:
            pQueueFamilyIndices[0] = queueFamilyIndices->graphicsFamily;
            break;
        default:
            VulkanLogError("Failed to create VkImageCreateInfoKHR due to invalid queueFamilyIndices!");
            return false;
    }

    const VkImageCreateInfo imageInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        NULL,
        0,
        VK_IMAGE_TYPE_2D,
        format,
        extent,
        mipmapLevels,
        1,
        samples,
        VK_IMAGE_TILING_OPTIMAL,
        usageFlags,
        queueFamilyIndices->familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices->familyCount,
        pQueueFamilyIndices,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    VulkanTest(vkCreateImage(device, &imageInfo, NULL, image), "Failed to create Vulkan %s image!", imageType);

    if (!imageMemory) return true; // If image memory is NULL, then allocation will be handled by the calling function
    // Otherwise, allocate the memory for the image

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, *image, &memoryRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memoryRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            const VkDeviceSize size = memoryRequirements.alignment * (VkDeviceSize) ceil(
                                          (double) memoryRequirements.size / (double) memoryRequirements.alignment);
            const VkMemoryAllocateInfo allocateInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                size,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocateInfo, NULL, imageMemory),
                       "Failed to allocate Vulkan %s image memory!", imageType);
            break;
        }
    }

    VulkanTest(vkBindImageMemory(device, *image, *imageMemory, 0), "Failed to bind Vulkan %s image memory!", imageType);

    return true;
}

static bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, const VkCommandPool commandPool)
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    VulkanTest(vkAllocateCommandBuffers(device, &allocateInfo, (VkCommandBuffer*)commandBuffer),
               "Failed to allocate Vulkan command buffers!");

    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        NULL
    };

    VulkanTest(vkBeginCommandBuffer(*commandBuffer, &beginInfo),
               "Failed to start the recording of Vulkan command buffers!");

    return true;
}

static bool EndCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandPool commandPool, const VkQueue queue)
{
    VulkanTest(vkEndCommandBuffer(commandBuffer), "Failed to finish the recording of Vulkan command buffers!");

    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        0,
        NULL,
        0,
        1,
        &commandBuffer,
        0,
        NULL
    };

    VulkanTest(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE),
               "Failed to submit Vulkan command buffers to queue!");

    VulkanTest(vkQueueWaitIdle(queue), "Failed to wait for Vulkan queue to become idle!");
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

    return true;
}

static bool CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usageFlags,
                         const VkMemoryPropertyFlags propertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    uint32_t pQueueFamilyIndices[queueFamilyIndices->familyCount];
    switch (queueFamilyIndices->familyCount)
    {
        case 3:
            pQueueFamilyIndices[2] = queueFamilyIndices->graphicsFamily;
        case 2:
            if (queueFamilyIndices->presentFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->transferFamily;
            }
            else if (queueFamilyIndices->transferFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->presentFamily;
            }
            else
            {
                VulkanLogError("Failed to create VkBufferCreateInfo due to invalid queueFamilyIndices!");
                return false;
            }
        case 1:
            pQueueFamilyIndices[0] = queueFamilyIndices->graphicsFamily;
            break;
        default:
            VulkanLogError("Failed to create VkBufferCreateInfo due to invalid queueFamilyIndices!");
            return false;
    }

    const VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usageFlags,
        queueFamilyIndices->familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices->familyCount,
        pQueueFamilyIndices
    };

    VulkanTest(vkCreateBuffer(device, &bufferInfo, NULL, buffer), "Failed to create Vulkan buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
        {
            const VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memRequirements.size,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, bufferMemory),
                       "Failed to allocate Vulkan buffer memory!");

            VulkanTest(vkBindBufferMemory(device, *buffer, *bufferMemory, 0), "Failed to bind Vulkan buffer memory!");

            return true;
        }
    }

    VulkanLogError("Failed to find suitable memory type for Vulkan!");

    return false;
}

static bool CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
    const VkCommandBuffer commandBuffer;
    if (!BeginCommandBuffer(&commandBuffer, transferCommandPool)) return false;

    const VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue)) return false;

    return true;
}

static void CleanupSwapChain()
{
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
    }
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
}

static void CleanupColorImage()
{
    vkDestroyImageView(device, colorImageView, NULL);
    vkDestroyImage(device, colorImage, NULL);
    vkFreeMemory(device, colorImageMemory, NULL);
}

static void CleanupDepthImage()
{
    vkDestroyImageView(device, depthImageView, NULL);
    vkDestroyImage(device, depthImage, NULL);
    vkFreeMemory(device, depthImageMemory, NULL);
}

static void CleanupSyncObjects()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);

        vkDestroyFence(device, inFlightFences[i], NULL);
    }
}

static bool RecreateSwapChain()
{
    VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for Vulkan device to become idle!");

    CleanupSwapChain();
    CleanupColorImage();
    CleanupDepthImage();
    CleanupSyncObjects();

    return CreateSwapChain() && CreateImageViews() && CreateColorImage() && CreateDepthImage() && CreateFramebuffers()
           && CreateSyncObjects();
}
#pragma region drawingHelpers
static void UpdateUniformBuffer(const uint32_t currentFrame)
{
    UniformBufferObject bufferObject = {
        GLM_MAT4_IDENTITY_INIT,
        GLM_MAT4_IDENTITY_INIT,
        GLM_MAT4_IDENTITY_INIT
    };
    mat4 ubo = GLM_MAT4_IDENTITY_INIT;

    glm_rotate(bufferObject.model, (float) SDL_GetTicks64() * PIf / 10000.0f, GLM_YUP);
    glm_lookat((vec3){2.0f, 2.0f, 2.0f}, GLM_VEC3_ZERO, (vec3){0.0f, -1.0f, 0.0f}, bufferObject.view);
    glm_perspective(PI / 4, (float) swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f,
                    bufferObject.proj);

    glm_mat4_mul(bufferObject.proj, bufferObject.view, ubo);
    glm_mat4_mul(ubo, bufferObject.model, ubo);

    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

static bool RecordCommandBuffer(const VkCommandBuffer buffer, const uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL
    };

    VulkanTest(vkBeginCommandBuffer(buffer, &beginInfo), "Failed to begin recording Vulkan command buffer!");

    const List(VkClearValue) clearColor = {
        2,
        (VkClearValue[]){
            {.color = {{0, 0, 0, 1}}},
            {.depthStencil = {1, 0}}
        }
    };

    const VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        renderPass,
        swapChainFramebuffers[imageIndex],
        {
            {0, 0},
            swapChainExtent
        },
        clearColor.length,
        clearColor.data
    };

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    const VkViewport viewport = {
        0,
        0,
        (float) swapChainExtent.width,
        (float) swapChainExtent.height,
        0,
        1
    };

    vkCmdSetViewport(buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
        {
            0,
            0
        },
        swapChainExtent
    };

    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdBindVertexBuffers(buffer, 0, 1, (VkBuffer[1]){vertexBuffer}, (VkDeviceSize[1]){0});
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &descriptorSets[currentFrame], 0, NULL);

    vkCmdDraw(buffer, vertices.length, 1, 0, 0);

    vkCmdEndRenderPass(buffer);

    VulkanTest(vkEndCommandBuffer(buffer), "Failed to record the Vulkan command buffer!");

    return true;
}
#pragma endregion drawingHelpers
#pragma endregion helperFunctions
static bool CreateInstance()
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
                          "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay. If you wish to disable the Mesa FPS overlay, that can be done by removing the definition for VK_ENABLE_MESA_FPS_OVERLAY in config.h");
        }
        FriendlyError("Missing Vulkan validation layers!",
                      "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\nYou can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package manager of your choice.\nIf you wish to disable the validation layer, that can be done by removing the definition for VK_ENABLE_VALIDATION_LAYER in config.h");
    }
    createInfo.enabledLayerCount = 2;
    createInfo.ppEnabledLayerNames = (const char *const[2]){"VK_LAYER_KHRONOS_validation", "VK_LAYER_MESA_overlay"};
#elifdef VK_ENABLE_VALIDATION_LAYER
    bool found = false;
    for (uint32_t i = 0; i < layerCount; i++)
    {
        if (!strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 28))
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        FriendlyError("Missing Vulkan validation layers!",
                      "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\nYou can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package manager of your choice.\nIf you wish to disable the validation layer, that can be done by removing the definition for VK_ENABLE_VALIDATION_LAYER in config.h");
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
                      "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay.\nIf you wish to disable the Mesa FPS overlay, that can be done by removing the definition for VK_ENABLE_MESA_FPS_OVERLAY in config.h");
    }
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_MESA_overlay"};
#endif

    VulkanTest(vkCreateInstance(&createInfo, NULL, &instance), "Failed to create Vulkan instance!")

    return true;
}

static bool CreateSurface()
{
    if (SDL_Vulkan_CreateSurface(vk_window, instance, &surface) == SDL_FALSE)
    {
        VulkanLogError("Failed to create Vulkan window surface");
        return false;
    }

    return true;
}

static bool PickPhysicalDevice()
{
    queueFamilyIndices = malloc(sizeof(*queueFamilyIndices));

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
        *queueFamilyIndices = (QueueFamilyIndices){-1, -1, -1, -1, 0};
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
                queueFamilyIndices->graphicsFamily = index;
                if (presentSupport) queueFamilyIndices->presentFamily = index;
            }
            else
            {
                if (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    // TODO investigate if separate transfer family is beneficial or detrimental
                    queueFamilyIndices->transferFamily = index;
                }
                if (presentSupport)
                {
                    queueFamilyIndices->uniquePresentFamily = index;
                }
            }

            if (queueFamilyIndices->graphicsFamily == -1 || queueFamilyIndices->transferFamily == -1 ||
                (queueFamilyIndices->presentFamily == -1 && queueFamilyIndices->uniquePresentFamily == -1))
            {
                continue;
            }

            break;
        }

        if (queueFamilyIndices->presentFamily == -1)
        {
            queueFamilyIndices->presentFamily = queueFamilyIndices->uniquePresentFamily;
        }
        if (queueFamilyIndices->transferFamily == -1)
        {
            queueFamilyIndices->transferFamily = queueFamilyIndices->graphicsFamily;
        }

        if (queueFamilyIndices->graphicsFamily == queueFamilyIndices->presentFamily ||
            queueFamilyIndices->graphicsFamily == queueFamilyIndices->transferFamily)
        {
            if (queueFamilyIndices->presentFamily == queueFamilyIndices->transferFamily)
            {
                queueFamilyIndices->familyCount = 1;
            }
            else
            {
                queueFamilyIndices->familyCount = 2;
            }
        }
        else
        {
            queueFamilyIndices->familyCount = 3;
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
                if (swapChainSupport->formatCount == 0 && swapChainSupport->presentModeCount == 0) continue;

                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
                if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    physicalDevice = devices[i];
                    return true;
                }
                physicalDevice = devices[i];
                match = true;
                break;
            }
        }
    }

    if (!match) { VulkanLogError("Failed to find a suitable GPU for Vulkan!"); }

    return match;
}

static bool CreateLogicalDevice()
{
    const float queuePriority = 1;

    VkDeviceQueueCreateInfo queueCreateInfo[queueFamilyIndices->familyCount];
    switch (queueFamilyIndices->familyCount)
    {
        case 3:
            queueCreateInfo[2] = (VkDeviceQueueCreateInfo){
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices->transferFamily,
                1,
                &queuePriority
            };
        case 2:
            queueCreateInfo[1] = (VkDeviceQueueCreateInfo){
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices->transferFamily,
                1,
                &queuePriority
            };
            if (queueFamilyIndices->transferFamily == queueFamilyIndices->graphicsFamily)
            {
                queueCreateInfo[1].queueFamilyIndex = queueFamilyIndices->presentFamily;
            }
            else if (queueFamilyIndices->presentFamily != queueFamilyIndices->graphicsFamily)
            {
                VulkanLogError("Failed to create VkDeviceQueueCreateInfo due to invalid queueFamilyIndices!");
                return false;
            }
        case 1:
            queueCreateInfo[0] = (VkDeviceQueueCreateInfo){
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                NULL,
                0,
                queueFamilyIndices->graphicsFamily,
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
        .runtimeDescriptorArray = VK_TRUE
    };
    VkDeviceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        &vulkan12Features,
        0,
        queueFamilyIndices->familyCount,
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

    VulkanTest(vkCreateDevice(physicalDevice, &createInfo, NULL, &device), "Failed to create Vulkan device!");

    vkGetDeviceQueue(device, queueFamilyIndices->graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices->transferFamily, 0, &transferQueue);
    vkGetDeviceQueue(device, queueFamilyIndices->presentFamily, 0, &presentQueue);

    return true;
}

static bool CreateSwapChain()
{
    if (minimized) return true;

    if (!QuerySwapChainSupport(physicalDevice))
    {
        VulkanLogError("Failed to query Vulkan swap chain support!");
        return false;
    }

    if (!swapChainSupport->capabilities.currentExtent.width || !swapChainSupport->capabilities.currentExtent.height)
    {
        // Window is minimized, so return to prevent creating a swap chain with dimensions of 0px by 0px
        // However, we do not want to fail or even log anything, since this is intended behavior
        minimized = true;
        return true;
    }

    VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_MAX_ENUM, VK_COLOR_SPACE_MAX_ENUM_KHR};
    for (uint32_t i = 0; i < swapChainSupport->formatCount; i++)
    {
        const VkSurfaceFormatKHR format = swapChainSupport->formats[i];
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB || surfaceFormat.format == VK_FORMAT_MAX_ENUM)
            {
                surfaceFormat = format;
            }
            else if (format.format == VK_FORMAT_R8G8B8A8_SRGB)
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

    VkExtent2D extent = swapChainSupport->capabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
    {
        int width, height;
        SDL_Vulkan_GetDrawableSize(vk_window, &width, &height);
        extent.width = clamp(width, swapChainSupport->capabilities.minImageExtent.width,
                             swapChainSupport->capabilities.maxImageExtent.width);
        extent.height = clamp(height, swapChainSupport->capabilities.minImageExtent.height,
                              swapChainSupport->capabilities.maxImageExtent.height);
    }
    uint32_t imageCount = swapChainSupport->capabilities.minImageCount + 1;
    if (swapChainSupport->capabilities.maxImageCount > 0 && imageCount > swapChainSupport->capabilities.maxImageCount)
    {
        imageCount = swapChainSupport->capabilities.maxImageCount;
    }

    uint32_t pQueueFamilyIndices[queueFamilyIndices->familyCount];
    switch (queueFamilyIndices->familyCount)
    {
        case 3:
            pQueueFamilyIndices[2] = queueFamilyIndices->graphicsFamily;
        case 2:
            if (queueFamilyIndices->presentFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->transferFamily;
            }
            else if (queueFamilyIndices->transferFamily == queueFamilyIndices->graphicsFamily)
            {
                pQueueFamilyIndices[1] = queueFamilyIndices->presentFamily;
            }
            else
            {
                VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!");
                return false;
            }
        case 1:
            pQueueFamilyIndices[0] = queueFamilyIndices->graphicsFamily;
            break;
        default:
            VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!");
            return false;
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for (uint32_t i = 0; i < swapChainSupport->presentModeCount; i++)
    {
        const VkPresentModeKHR mode = swapChainSupport->presentMode[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            break;
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
        queueFamilyIndices->familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices->familyCount,
        pQueueFamilyIndices,
        swapChainSupport->capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    VulkanTest(vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain), "Failed to create Vulkan swap chain!");

    VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL), "Failed to get Vulkan swapchain images!");
    swapChainImages = malloc(sizeof(*swapChainImages) * imageCount);
    swapChainCount = imageCount;
    VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages),
               "Failed to get Vulkan swapchain images!");
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return true;
}

static bool CreateImageViews()
{
    swapChainImageViews = malloc(sizeof(*swapChainImageViews) * swapChainCount);

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

static bool CreateRenderPass()
{
    // TODO if stencil is not needed then allow for using VK_FORMAT_D32_SFLOAT
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, &properties);
    if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        depthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    else
    {
        vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, &properties);
        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthImageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        }
        else
        {
            VulkanLogError("Unable to find suitable format for Vulkan depth image!");
            return false;
        }
    }

    const List(VkAttachmentDescription) attachments = {
        3,
        (VkAttachmentDescription[]){
            {
                0,
                swapChainImageFormat,
                MSAA_SAMPLES,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            },
            {
                0,
                depthImageFormat,
                MSAA_SAMPLES,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            },
            {
                0,
                swapChainImageFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            }
        }
    };

    VkAttachmentReference colorAttachmentRef = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentReference = {
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference colorAttachmentResolveRef = {
        2,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {
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

    VkSubpassDependency dependency = {
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        0
    };

    const VkRenderPassCreateInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        attachments.length,
        attachments.data,
        1,
        &subpass,
        1,
        &dependency
    };

    VulkanTest(vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass), "Failed to create Vulkan render pass!");

    return true;
}

static bool CreateDescriptorSetLayout()
{
    const List(VkDescriptorSetLayoutBinding) bindings = {
        3,
        (VkDescriptorSetLayoutBinding[]){
            {
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                NULL
            },
            {
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                TEXTURE_ASSET_COUNT,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                NULL
            },
            {
                2,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                1,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                NULL
            }
        }
    };
    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        NULL,
        0,
        bindings.length,
        bindings.data
    };

    VulkanTest(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout),
               "Failed to create Vulkan descriptor set layout!");

    return true;
}

static bool CreateGraphicsPipeline()
{
    VkShaderModule vertShaderModule = CreateShaderModule((uint32_t *) DecompressAsset(gzvert_Vulkan_basic),
                                                         AssetGetSize(gzvert_Vulkan_basic));
    VkShaderModule fragShaderModule = CreateShaderModule((uint32_t *) DecompressAsset(gzfrag_Vulkan_basic),
                                                         AssetGetSize(gzfrag_Vulkan_basic));
    VkShaderModule tescShaderModule = CreateShaderModule((uint32_t *) DecompressAsset(gztesc_Vulkan_test),
                                                         AssetGetSize(gztesc_Vulkan_test));
    VkShaderModule teseShaderModule = CreateShaderModule((uint32_t *) DecompressAsset(gztese_Vulkan_test),
                                                         AssetGetSize(gztese_Vulkan_test));
    if (!vertShaderModule || !fragShaderModule || !tescShaderModule || !teseShaderModule)
    {
        VulkanLogError("Failed to load Vulkan shaders!");
        return false;
    }

    VkPipelineShaderStageCreateInfo shaderStages[4] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShaderModule,
            "main",
            NULL
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShaderModule,
            "main",
            NULL
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            tescShaderModule,
            "main",
            NULL
        },
    {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            NULL,
            0,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            teseShaderModule,
            "main",
            NULL
        }
    };

    const VkVertexInputBindingDescription bindingDescription = {
        0,
        sizeof(Vertex),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    const List(VkVertexInputAttributeDescription) vertexDescriptions = {
        3,
        (VkVertexInputAttributeDescription[]){
            {
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, pos)
            },
            {
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, color)
            },
            {
                2,
                0,
                VK_FORMAT_R32G32_SFLOAT,
                offsetof(Vertex, textureCoordinate)
            }
        }
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &bindingDescription,
        vertexDescriptions.length,
        vertexDescriptions.data
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        NULL,
        0,
        VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
        VK_FALSE
    };

    VkPipelineTessellationStateCreateInfo tessellationState = {
        VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        NULL,
        0,
        4
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        NULL,
        1,
        NULL
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
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

    VkPipelineMultisampleStateCreateInfo multisampling = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        NULL,
        0,
        MSAA_SAMPLES,
        VK_FALSE,
        1,
        NULL,
        VK_FALSE,
        VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil = {
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

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_TRUE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        NULL,
        0,
        VK_TRUE,
        VK_LOGIC_OP_COPY,
        1,
        &colorBlendAttachment,
        {0, 0, 0, 0}
    };

    List(VkDynamicState) dynamicStates = {2, (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}};
    VkPipelineDynamicStateCreateInfo dynamicState = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        NULL,
        0,
        dynamicStates.length,
        dynamicStates.data
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &descriptorSetLayout,
        0,
        NULL
    };
    VulkanTest(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout),
               "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        NULL,
        0,
        4,
        shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        &tessellationState,
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

    VulkanTest(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline),
               "Failed to create graphics pipeline!");

    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
    vkDestroyShaderModule(device, tescShaderModule, NULL);
    vkDestroyShaderModule(device, teseShaderModule, NULL);

    return true;
}

static bool CreateCommandPools()
{
    const VkCommandPoolCreateInfo graphicsPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndices->graphicsFamily
    };

    VulkanTest(vkCreateCommandPool(device, &graphicsPoolInfo, NULL, &graphicsCommandPool),
               "Failed to create Vulkan graphics command pool!");

    const VkCommandPoolCreateInfo transferPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndices->transferFamily
    };

    VulkanTest(vkCreateCommandPool(device, &transferPoolInfo, NULL, &transferCommandPool),
               "Failed to create Vulkan transfer command pool!");

    return true;
}

static bool CreateColorImage()
{
    if (!CreateImage(&colorImage, &colorImageMemory, swapChainImageFormat,
                     (VkExtent3D){swapChainExtent.width, swapChainExtent.height, 1}, 1,
                     MSAA_SAMPLES, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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

static bool CreateDepthImage()
{
    if (!CreateImage(&depthImage, &depthImageMemory, depthImageFormat,
                     (VkExtent3D){swapChainExtent.width, swapChainExtent.height, 1}, 1,
                     MSAA_SAMPLES, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "depth test"))
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

static bool CreateFramebuffers()
{
    swapChainFramebuffers = malloc(sizeof(*swapChainFramebuffers) * swapChainCount);

    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        const List(VkImageView) attachments = {
            3,
            (VkImageView[]){colorImageView, depthImageView, swapChainImageViews[i]}
        };

        VkFramebufferCreateInfo framebufferInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            renderPass,
            attachments.length,
            attachments.data,
            swapChainExtent.width,
            swapChainExtent.height,
            1
        };

        VulkanTest(vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]),
                   "Failed to create Vulkan framebuffer!");
    }

    return true;
}

static bool LoadTextures()
{
    VkDeviceSize memorySize = 0;
    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        const uint8_t *decompressed = DecompressAsset(texture_assets[textureIndex]);

        const VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            VulkanLogError("Vulkan texture image format does not support linear blitting!");
            return false;
        }

        const VkExtent3D extent = {ReadUintA(decompressed, 4), ReadUintA(decompressed, 8), 1};
        textures[textureIndex].mipmapLevels = (uint8_t) log2(max(extent.width, extent.height)) + 1;
        if (!CreateImage(&textures[textureIndex].image, NULL, format, extent, textures[textureIndex].mipmapLevels,
                         VK_SAMPLE_COUNT_1_BIT,
                         VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                         "texture"))
        {
            return false;
        }

        vkGetImageMemoryRequirements(device, textures[textureIndex].image, &textures[textureIndex].memoryRequirements);
        textures[textureIndex].offset = textures[textureIndex].memoryRequirements.alignment * (VkDeviceSize) ceil(
                                            ((double) memorySize + (double) textures[textureIndex].memoryRequirements.
                                             size) / (double) textures[textureIndex].memoryRequirements.alignment);
        memorySize = textures[textureIndex].offset + textures[textureIndex].memoryRequirements.size;

        texturesAssetIDMap[ReadUintA(decompressed, 12)] = textureIndex;
    }

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (textures[i].memoryRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
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
    VkDeviceMemory stagingBufferMemory;
    if (!CreateBuffer(memorySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                      &stagingBufferMemory))
    {
        return false;
    }

    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        VulkanTest(
            vkBindImageMemory(device, textures[textureIndex].image, textureMemory, textures[textureIndex].offset),
            "Failed to bind Vulkan texture memory!");

        const uint8_t *decompressed = DecompressAsset(texture_assets[textureIndex]);
        uint32_t width = ReadUintA(decompressed, 4);
        uint32_t height = ReadUintA(decompressed, 8);
        void *data;
        data = calloc(1, textures[textureIndex].memoryRequirements.size);

        VulkanTest(
            vkMapMemory(device, stagingBufferMemory, textures[textureIndex].offset, textures[textureIndex].
                memoryRequirements.size, 0, &data), "Failed to map Vulkan texture staging buffer memory!");

        memcpy(data, decompressed + sizeof(uint32_t) * 4, ReadUintA(decompressed, 0) * 4);
        vkUnmapMemory(device, stagingBufferMemory);

        const VkCommandBuffer commandBuffer;
        if (!BeginCommandBuffer(&commandBuffer, transferCommandPool)) return false;

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

        if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue)) return false;

        if (!BeginCommandBuffer(&commandBuffer, transferCommandPool)) return false;

        const VkBufferImageCopy bufferCopyInfo = {
            textures[textureIndex].offset,
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

        if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue)) return false;

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
                    {(int32_t) width, (int32_t) height, 1}
                },
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    level + 1,
                    0,
                    1
                },
                {
                    {0, 0, 0},
                    {width > 1 ? (int32_t) width / 2 : 1, height > 1 ? (int32_t) height / 2 : 1, 1}
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
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return true;
}

static bool CreateTexturesImageView()
{
    for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
    {
        if (!CreateImageView(&texturesImageView[textureIndex], textures[textureIndex].image, VK_FORMAT_R8G8B8A8_SRGB,
                             VK_IMAGE_ASPECT_COLOR_BIT, textures[textureIndex].mipmapLevels,
                             "Failed to create Vulkan texture image view!"))
        {
            return false;
        }
    }

    return true;
}

static bool CreateTextureSampler()
{
    const VkSamplerCreateInfo samplerCreateInfo = {
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

    VulkanTest(vkCreateSampler(device, &samplerCreateInfo, NULL, &textureSampler),
               "Failed to create Vulkan texture sampler!");

    return true;
}

static bool CreateVertexBuffer()
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    void *data;

    const VkDeviceSize bufferSize = sizeof(vertices.data[0]) * vertices.length;

    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                      &stagingBufferMemory))
    {
        return false;
    }

    VulkanTest(vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data),
               "Failed to map Vulkan vertex staging buffer memory!");

    memcpy(data, vertices.data, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory))
    {
        return false;
    }

    if (!CopyBuffer(stagingBuffer, vertexBuffer, bufferSize)) return false;

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    return true;
}

static bool CreateUniformBuffers()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        const VkDeviceSize uniformBufferSize = sizeof(mat4);

        if (!CreateBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          &uniformBuffers[i], &uniformBuffersMemory[i]))
            return false;

        VulkanTest(vkMapMemory(device, uniformBuffersMemory[i], 0, uniformBufferSize, 0, &uniformBuffersMapped[i]),
                   "Failed to map Vulkan uniform buffer memory!");
    }

    const VkDeviceSize dataBufferSize = sizeof(DataBufferObject);

    if (!CreateBuffer(dataBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &dataBuffer,
                      &dataBufferMemory))
        return false;

    VulkanTest(vkMapMemory(device, dataBufferMemory, 0, dataBufferSize, 0, &mappedDataBuffer),
               "Failed to map Vulkan data buffer memory!");

    return true;
}

static bool CreateDescriptorPool()
{
    const List(VkDescriptorPoolSize) poolSizes = {
        3,
        (VkDescriptorPoolSize[]){
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                MAX_FRAMES_IN_FLIGHT * MAX_FRAMES_IN_FLIGHT
            },
            {
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                MAX_FRAMES_IN_FLIGHT * TEXTURE_ASSET_COUNT
            },
            {
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                MAX_FRAMES_IN_FLIGHT
            }
        }
    };

    const VkDescriptorPoolCreateInfo poolCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0,
        MAX_FRAMES_IN_FLIGHT,
        poolSizes.length,
        poolSizes.data
    };

    VulkanTest(vkCreateDescriptorPool(device, &poolCreateInfo, NULL, &descriptorPool),
               "Failed to create Vulkan descriptor pool!");

    return true;
}

static bool CreateDescriptorSets()
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

    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo = {
            uniformBuffers[i],
            0,
            sizeof(mat4)
        };

        VkDescriptorImageInfo imageInfo[TEXTURE_ASSET_COUNT];
        for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
        {
            imageInfo[textureIndex] = (VkDescriptorImageInfo){
                textureSampler,
                texturesImageView[textureIndex],
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }

        VkDescriptorBufferInfo dataBufferInfo = {
            dataBuffer,
            0,
            sizeof(DataBufferObject)
        };

        const List(VkWriteDescriptorSet) writeDescriptorList = {
            3,
            (VkWriteDescriptorSet[]){
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    NULL,
                    descriptorSets[i],
                    0,
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    NULL,
                    &uniformBufferInfo,
                    NULL
                },
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    NULL,
                    descriptorSets[i],
                    1,
                    0,
                    TEXTURE_ASSET_COUNT,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    imageInfo,
                    NULL,
                    NULL
                },
                {
                    VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    NULL,
                    descriptorSets[i],
                    2,
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    NULL,
                    &dataBufferInfo,
                    NULL
                }
            }
        };
        vkUpdateDescriptorSets(device, writeDescriptorList.length, writeDescriptorList.data, 0, NULL);
    }

    return true;
}

static bool CreateCommandBuffers()
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

static bool CreateSyncObjects()
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
#pragma endregion internalFunctions

bool VK_Init(SDL_Window *window)
{
    vk_window = window;
    if (CreateInstance() && CreateSurface() && PickPhysicalDevice() && CreateLogicalDevice() && CreateSwapChain() &&
        CreateImageViews() && CreateRenderPass() && CreateDescriptorSetLayout() && CreateGraphicsPipeline() &&
        CreateCommandPools() && CreateColorImage() && CreateDepthImage() && CreateFramebuffers() && LoadTextures() &&
        CreateTexturesImageView() && CreateTextureSampler() && CreateVertexBuffer() && CreateUniformBuffers() &&
        CreateDescriptorPool() && CreateDescriptorSets() && CreateCommandBuffers() && CreateSyncObjects())
    {
        const DataBufferObject dataBufferObject = {
            texturesAssetIDMap[ReadUintA(DecompressAsset(gztex_actor_coin), 12)]
        };

        memcpy(mappedDataBuffer, &dataBufferObject, sizeof(dataBufferObject));

        return true;
    }
    VK_Cleanup();

    return false;
}

VkResult VK_FrameStart()
{
    if (minimized) return VK_SUCCESS;

    VulkanTestReturnResult(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX),
                           "Failed to wait for Vulkan fences!");

    VulkanTestReturnResult(vkResetFences(device, 1, &inFlightFences[currentFrame]), "Failed to reset Vulkan fences!");

    const VkResult acquireNextImageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                                                                  imageAvailableSemaphores[currentFrame],
                                                                  VK_NULL_HANDLE, &swapchainImageIndex);

    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireNextImageResult == VK_SUBOPTIMAL_KHR)
    {
        if (RecreateSwapChain()) return VK_SUCCESS;
    }
    VulkanTestReturnResult(acquireNextImageResult, "Failed to acquire next Vulkan image index!");

    VulkanTestReturnResult(vkResetCommandBuffer(commandBuffers[currentFrame], 0),
                           "Failed to reset Vulkan command buffer!");

    return VK_SUCCESS;
}

VkResult VK_FrameEnd()
{
    if (!RecordCommandBuffer(commandBuffers[currentFrame], swapchainImageIndex)) return VK_ERROR_UNKNOWN;

    const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        waitSemaphores,
        waitStages,
        1,
        &commandBuffers[currentFrame],
        1,
        signalSemaphores
    };

    VulkanTestReturnResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),
                           "Failed to submit Vulkan draw command buffer!");

    const VkSwapchainKHR swapChains[] = {swapChain};
    const VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        signalSemaphores,
        1,
        swapChains,
        &swapchainImageIndex,
        NULL
    };

    const VkResult queuePresentResult = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR)
    {
        if (RecreateSwapChain()) return VK_SUCCESS;
    }
    VulkanTestReturnResult(queuePresentResult, "Failed to queue frame for presentation!");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return VK_SUCCESS;
}

VkResult VK_RenderLevel()
{
    UpdateUniformBuffer(currentFrame);
    return VK_SUCCESS;
}

bool VK_Cleanup()
{
    if (device)
    {
        VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for Vulkan device to become idle!");

        CleanupSwapChain();

        vkDestroySampler(device, textureSampler, NULL);
        for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
        {
            vkDestroyImageView(device, texturesImageView[textureIndex], NULL);
            vkDestroyImage(device, textures[textureIndex].image, NULL);
        }
        vkFreeMemory(device, textureMemory, NULL);

        CleanupColorImage();
        CleanupDepthImage();

        vkDestroyPipeline(device, graphicsPipeline, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);

        vkDestroyRenderPass(device, renderPass, NULL);

        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(device, uniformBuffers[i], NULL);
            vkFreeMemory(device, uniformBuffersMemory[i], NULL);
        }

        vkDestroyBuffer(device, dataBuffer, NULL);
        vkFreeMemory(device, dataBufferMemory, NULL);

        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);

        vkDestroyBuffer(device, indexBuffer, NULL);
        vkFreeMemory(device, indexBufferMemory, NULL);

        vkDestroyBuffer(device, vertexBuffer, NULL);
        vkFreeMemory(device, vertexBufferMemory, NULL);

        CleanupSyncObjects();

        vkDestroyCommandPool(device, graphicsCommandPool, NULL);
        vkDestroyCommandPool(device, transferCommandPool, NULL);
    }

    vkDestroyDevice(device, NULL);

    if (instance) vkDestroySurfaceKHR(instance, surface, NULL);

    vkDestroyInstance(instance, NULL);

    return true;
}

inline void VK_Minimize()
{
    minimized = true;
}

inline void VK_Restore()
{
    minimized = false;
}

inline VkSampleCountFlags VK_MaxSampleCount()
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    return properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
}
