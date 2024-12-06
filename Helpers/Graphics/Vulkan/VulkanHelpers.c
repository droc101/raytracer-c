//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"

#pragma region variables
SDL_Window *vk_window;
bool minimized = false;

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface;
PhysicalDevice physicalDevice;
QueueFamilyIndices queueFamilyIndices;
SwapChainSupportDetails swapChainSupport;
VkDevice device = NULL;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSwapchainKHR swapChain = VK_NULL_HANDLE;
VkImage *swapChainImages;
uint32_t swapChainCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkPipelineCache pipelineCache = VK_NULL_HANDLE;
Pipelines pipelines;
VkFramebuffer *swapChainFramebuffers;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
bool framebufferResized = false;
uint8_t currentFrame = 0;
uint32_t swapchainImageIndex;
MemoryPools memoryPools = {
    {
        0,
        VK_NULL_HANDLE,
        0,
        0,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    },
    {
        0,
        VK_NULL_HANDLE,
        0,
        0,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    }
};
Buffers buffers = {0};
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
Image textures[TEXTURE_ASSET_COUNT];
VkDeviceMemory textureMemory;
VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
uint32_t texturesAssetIDMap[ASSET_COUNT];
TextureSamplers textureSamplers;
VkFormat depthImageFormat;
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;
VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;
VkClearColorValue clearColor = {{0.0f, 0.64f, 0.91f, 1.0f}};
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
#pragma endregion variables

bool QuerySwapChainSupport(const VkPhysicalDevice pDevice)
{
    SwapChainSupportDetails details = {0, 0, NULL, NULL, {}};

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
    swapChainSupport = details;

    return true;
}

bool CreateImageView(VkImageView *imageView,
                     const VkImage image,
                     const VkFormat format,
                     const VkImageAspectFlagBits aspectMask,
                     const uint8_t mipmapLevels,
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

VkShaderModule CreateShaderModule(const uint32_t *code, const size_t size)
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

bool CreateImage(VkImage *image,
                 VkDeviceMemory *imageMemory,
                 const VkFormat format,
                 const VkExtent3D extent,
                 const uint8_t mipmapLevels,
                 const VkSampleCountFlags samples,
                 const VkImageUsageFlags usageFlags,
                 const char *imageType)
{
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
        queueFamilyIndices.familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices.familyCount,
        pQueueFamilyIndices,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    VulkanTest(vkCreateImage(device, &imageInfo, NULL, image), "Failed to create Vulkan %s image!", imageType);

    if (!imageMemory) return true; // If image memory is NULL, then allocation will be handled by the calling function
    // Otherwise, allocate the memory for the image

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, *image, &memoryRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.device, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memoryRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        {
            const VkDeviceSize size = memoryRequirements.alignment * (VkDeviceSize)ceil(
                                          (double)memoryRequirements.size / (double)memoryRequirements.alignment);
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

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, const VkCommandPool commandPool)
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

bool EndCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandPool commandPool, const VkQueue queue)
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

bool CreateBuffer(VkBuffer *buffer,
                  const VkDeviceSize size,
                  const VkBufferUsageFlags usageFlags,
                  const bool newAllocation,
                  MemoryAllocationInfo *allocationInfo)
{
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
            VulkanLogError("Failed to create VkBufferCreateInfo due to invalid queueFamilyIndices!");
            return false;
    }

    const VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usageFlags,
        queueFamilyIndices.familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        queueFamilyIndices.familyCount,
        pQueueFamilyIndices
    };

    VulkanTest(vkCreateBuffer(device, &bufferInfo, NULL, buffer), "Failed to create Vulkan buffer!");

    vkGetBufferMemoryRequirements(device, *buffer, &allocationInfo->memoryRequirements);
    const VkDeviceSize memorySize = allocationInfo->memoryRequirements.alignment * (VkDeviceSize)ceil(
                                        (double)allocationInfo->memoryRequirements.size / (double)allocationInfo->
                                        memoryRequirements.alignment);

    allocationInfo->offset = allocationInfo->memoryInfo->size;
    allocationInfo->memoryInfo->size += memorySize;
    allocationInfo->memoryInfo->memoryTypeBits |= allocationInfo->memoryRequirements.memoryTypeBits;

    if (!newAllocation) return true; // Allocation and binding will be handled elsewhere

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.device, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (allocationInfo->memoryRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & allocationInfo->memoryInfo->type) == allocationInfo->
            memoryInfo->type)
        {
            const VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memorySize,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &allocationInfo->memoryInfo->memory),
                       "Failed to allocate Vulkan buffer memory!");

            VulkanTest(vkBindBufferMemory(device, *buffer, allocationInfo->memoryInfo->memory, 0),
                       "Failed to bind Vulkan buffer memory!");

            return true;
        }
    }

    VulkanLogError("Failed to find suitable memory type for buffer!");

    return false;
}

bool CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
    const VkCommandBuffer commandBuffer;
    if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool)) return false;

    const VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue)) return false;

    return true;
}

bool AllocateMemory()
{
    bool allocated = false;

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.device, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memoryPools.localMemory.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & memoryPools.localMemory.type) == memoryPools.localMemory.
            type)
        {
            const VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memoryPools.localMemory.size,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &memoryPools.localMemory.memory),
                       "Failed to allocate device local buffer memory!");

            allocated = true;
            break;
        }
    }
    if (!allocated)
    {
        VulkanLogError("Failed to allocate device local buffer memory!");

        return false;
    }

    allocated = false;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memoryPools.sharedMemory.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & memoryPools.sharedMemory.type) == memoryPools.sharedMemory.
            type)
        {
            const VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memoryPools.sharedMemory.size,
                i
            };

            VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &memoryPools.sharedMemory.memory),
                       "Failed to allocate device shared buffer memory!");

            allocated = true;
            break;
        }
    }
    if (!allocated)
    {
        VulkanLogError("Failed to allocate device shared buffer memory!");

        return false;
    }

    VulkanTest(
        vkBindBufferMemory(device, buffers.ui.buffer, memoryPools.sharedMemory.memory, buffers.ui.memoryAllocationInfo.
            offset), "Failed to bind UI vertex buffer memory!");
    VulkanTest(
        vkBindBufferMemory(device, buffers.data.buffer, memoryPools.sharedMemory.memory, buffers.data.
            memoryAllocationInfo.offset), "Failed to bind data uniform buffer memory!");
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VulkanTest(
            vkBindBufferMemory(device, buffers.translation[i].buffer, memoryPools.sharedMemory.memory, buffers.
                translation[i].memoryAllocationInfo.offset), "Failed to bind translation uniform buffer memory!");
    }

    VulkanTest(
        vkBindBufferMemory(device, buffers.walls.buffer, memoryPools.localMemory.memory, buffers.walls.
            memoryAllocationInfo.offset), "Failed to bind Vulkan buffer memory!");

    VulkanTest(
        vkMapMemory(device, memoryPools.sharedMemory.memory, 0, VK_WHOLE_SIZE, 0, &memoryPools.sharedMemory.mappedMemory
        ), "Failed to map Vulkan buffer memory!");

    buffers.ui.vertices = memoryPools.sharedMemory.mappedMemory + buffers.ui.memoryAllocationInfo.offset;
    buffers.data.data = memoryPools.sharedMemory.mappedMemory + buffers.data.memoryAllocationInfo.offset;
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        buffers.translation[i].data = memoryPools.sharedMemory.mappedMemory + buffers.translation[i].
                                                                              memoryAllocationInfo.offset;
    }

    VkBuffer stagingBuffer;
    void *data;
    const WallVertex vertices[8] = {
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f}},
        {{0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}},
        {{0.5f, 0.0f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}}
    };

    const VkDeviceSize bufferSize = sizeof(*vertices) * 8;

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

    if (!CreateBuffer(&stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, &allocationInfo))
    {
        return false;
    }

    VulkanTest(vkMapMemory(device, memoryInfo.memory, 0, bufferSize, 0, &data),
               "Failed to map Vulkan vertex staging buffer memory!");

    memcpy(data, vertices, bufferSize);
    vkUnmapMemory(device, memoryInfo.memory);


    if (!CopyBuffer(stagingBuffer, buffers.walls.buffer, bufferSize)) return false;
    buffers.walls.vertexCount = 8;

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, memoryInfo.memory, NULL);

    return true;
}

void CleanupSwapChain()
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

void CleanupColorImage()
{
    vkDestroyImageView(device, colorImageView, NULL);
    vkDestroyImage(device, colorImage, NULL);
    vkFreeMemory(device, colorImageMemory, NULL);
}

void CleanupDepthImage()
{
    vkDestroyImageView(device, depthImageView, NULL);
    vkDestroyImage(device, depthImage, NULL);
    vkFreeMemory(device, depthImageMemory, NULL);
}

void CleanupPipeline()
{
    vkDestroyPipeline(device, pipelines.walls, NULL);
    vkDestroyPipeline(device, pipelines.ui, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
}

void CleanupSyncObjects()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);

        vkDestroyFence(device, inFlightFences[i], NULL);
    }
}

void UpdateUniformBuffer(const uint32_t currentFrame)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    mat4 proj = GLM_MAT4_IDENTITY_INIT;
    mat4 ubo = GLM_MAT4_IDENTITY_INIT;

    glm_rotate(model, (float)SDL_GetTicks64() * PIf / 10000.0f, GLM_YUP);
    glm_lookat((vec3){2.0f, 2.0f, 2.0f}, GLM_VEC3_ZERO, (vec3){0.0f, -1.0f, 0.0f}, view);
    glm_perspective(PI / 4, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f, proj);

    glm_mat4_mul(proj, view, ubo);
    glm_mat4_mul(ubo, model, ubo);

    memcpy(buffers.translation[currentFrame].data, &ubo, sizeof(ubo));
}

VkResult BeginRenderPass(const VkCommandBuffer commandBuffer, const uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        NULL
    };

    VulkanTestReturnResult(vkBeginCommandBuffer(commandBuffer, &beginInfo),
                           "Failed to begin recording Vulkan command buffer!");

    const VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        renderPass,
        swapChainFramebuffers[imageIndex],
        {
            {0, 0},
            swapChainExtent
        },
        2,
        (VkClearValue[]){{.color = clearColor}, {.depthStencil = {1, 0}}}
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return VK_SUCCESS;
}

VkResult EndRenderPass(const VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);

    VulkanTestReturnResult(vkEndCommandBuffer(commandBuffer), "Failed to record the Vulkan command buffer!");

    return VK_SUCCESS;
}

void DrawVertexBuffer(const VkCommandBuffer commandBuffer,
                      const VkPipeline pipeline,
                      const VertexBuffer vertexBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, (VkBuffer[1]){vertexBuffer.buffer}, (VkDeviceSize[1]){0});

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &descriptorSets[currentFrame], 0, NULL);

    vkCmdDraw(commandBuffer, vertexBuffer.vertexCount, 1, 0, 0);
}

bool DrawRectInternal(const float ndcStartX,
                      const float ndcStartY,
                      const float ndcEndX,
                      const float ndcEndY,
                      const float startU,
                      const float startV,
                      const float endU,
                      const float endV,
                      const uint32_t color,
                      const uint32_t textureIndex)
{
    return DrawQuadInternal((mat4){
                                {ndcStartX, ndcStartY, startU, startV},
                                {ndcEndX, ndcStartY, endU, startV},
                                {ndcEndX, ndcEndY, endU, endV},
                                {ndcStartX, ndcEndY, startU, endV}
                            }, color, textureIndex);
}

bool DrawQuadInternal(const mat4 vertices_posXY_uvZW,
                      const uint32_t color,
                      const uint32_t textureIndex)
{
    GET_COLOR(color);

    if (buffers.ui.vertexCount >= buffers.ui.maxVertices)
    {
        if (buffers.ui.vertexCount >= buffers.ui.fallbackMaxVertices)
        {
            if (buffers.ui.fallbackMaxVertices)
            {
                buffers.ui.fallbackMaxVertices += 64;
                UiVertex *newVertices = realloc(buffers.ui.fallback,
                                                sizeof(UiVertex) * buffers.ui.fallbackMaxVertices);
                if (!newVertices)
                {
                    free(newVertices);
                    free(buffers.ui.fallback);
                    VulkanLogError("realloc of fallback UI vertex buffer failed!");
                    return false;
                }
                buffers.ui.fallback = newVertices;
            } else
            {
                buffers.ui.fallbackMaxVertices = buffers.ui.maxVertices + 64;
                buffers.ui.fallback = malloc(sizeof(UiVertex) * buffers.ui.fallbackMaxVertices);
                memcpy(buffers.ui.fallback, buffers.ui.vertices,
                       sizeof(UiVertex) * buffers.ui.maxVertices);
                if (!buffers.ui.fallback)
                {
                    VulkanLogError("malloc of fallback UI vertex buffer failed!");
                    return false;
                }
            }
        }
        buffers.ui.fallback[buffers.ui.vertexCount++] = (UiVertex){
            {
                vertices_posXY_uvZW[0][0],
                vertices_posXY_uvZW[0][1],
                vertices_posXY_uvZW[0][2],
                vertices_posXY_uvZW[0][3]
            },
            {r, g, b, a},
            textureIndex
        };
        buffers.ui.fallback[buffers.ui.vertexCount++] = (UiVertex){
            {
                vertices_posXY_uvZW[1][0],
                vertices_posXY_uvZW[1][1],
                vertices_posXY_uvZW[1][2],
                vertices_posXY_uvZW[1][3]
            },
            {r, g, b, a},
            textureIndex
        };
        buffers.ui.fallback[buffers.ui.vertexCount++] = (UiVertex){
            {
                vertices_posXY_uvZW[2][0],
                vertices_posXY_uvZW[2][1],
                vertices_posXY_uvZW[2][2],
                vertices_posXY_uvZW[2][3]
            },
            {r, g, b, a},
            textureIndex
        };
        buffers.ui.fallback[buffers.ui.vertexCount++] = (UiVertex){
            {
                vertices_posXY_uvZW[3][0],
                vertices_posXY_uvZW[3][1],
                vertices_posXY_uvZW[3][2],
                vertices_posXY_uvZW[3][3]
            },
            {r, g, b, a},
            textureIndex
        };

        return true;
    }

    buffers.ui.vertices[buffers.ui.vertexCount++] = (UiVertex){
        {
            vertices_posXY_uvZW[0][0],
            vertices_posXY_uvZW[0][1],
            vertices_posXY_uvZW[0][2],
            vertices_posXY_uvZW[0][3]
        },
        {r, g, b, a},
        textureIndex
    };
    buffers.ui.vertices[buffers.ui.vertexCount++] = (UiVertex){
        {
            vertices_posXY_uvZW[1][0],
            vertices_posXY_uvZW[1][1],
            vertices_posXY_uvZW[1][2],
            vertices_posXY_uvZW[1][3]
        },
        {r, g, b, a},
        textureIndex
    };
    buffers.ui.vertices[buffers.ui.vertexCount++] = (UiVertex){
        {
            vertices_posXY_uvZW[2][0],
            vertices_posXY_uvZW[2][1],
            vertices_posXY_uvZW[2][2],
            vertices_posXY_uvZW[2][3]
        },
        {r, g, b, a},
        textureIndex
    };
    buffers.ui.vertices[buffers.ui.vertexCount++] = (UiVertex){
        {
            vertices_posXY_uvZW[3][0],
            vertices_posXY_uvZW[3][1],
            vertices_posXY_uvZW[3][2],
            vertices_posXY_uvZW[3][3]
        },
        {r, g, b, a},
        textureIndex
    };

    return true;
}
