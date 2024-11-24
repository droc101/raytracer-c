//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"

#pragma region variables
SDL_Window *vk_window;
bool minimized = false;

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface;
VkPhysicalDevice physicalDevice;
QueueFamilyIndices *queueFamilyIndices;
SwapChainSupportDetails *swapChainSupport;
VkDevice device = NULL;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkQueue transferQueue;
VkSwapchainKHR swapChain = VK_NULL_HANDLE;
VkImage *swapChainImages;
uint32_t swapChainCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
Pipelines pipelines;
VkFramebuffer *swapChainFramebuffers;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
bool framebufferResized = false;
uint8_t currentFrame = 0;
uint32_t swapchainImageIndex;
VertexBuffers vertexBuffers = {NULL};
VkBuffer indexBuffer = VK_NULL_HANDLE;
VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
ImageAllocationInformation textures[TEXTURE_ASSET_COUNT];
VkDeviceMemory textureMemory;
VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
uint16_t texturesAssetIDMap[ASSET_COUNT];
VkSampler textureSampler;
VkBuffer dataBuffer = VK_NULL_HANDLE;
VkDeviceMemory dataBufferMemory = VK_NULL_HANDLE;
void *mappedDataBuffer;
VkFormat depthImageFormat;
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;
VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;
#pragma endregion variables

bool QuerySwapChainSupport(const VkPhysicalDevice pDevice)
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

bool CreateImageView(VkImageView *imageView, const VkImage image, const VkFormat format,
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

bool CreateImage(VkImage *image, VkDeviceMemory *imageMemory, const VkFormat format, const VkExtent3D extent,
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

bool CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usageFlags,
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

    if (!bufferMemory) return true; // Allocation and binding will be handled by the calling function

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

    VulkanLogError("Failed to find suitable memory type for buffer!");

    return false;
}

bool CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
    const VkCommandBuffer commandBuffer;
    if (!BeginCommandBuffer(&commandBuffer, transferCommandPool)) return false;

    const VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue)) return false;

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

VkResult BeginRenderPass(const VkCommandBuffer commandBuffer, const uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL
    };

    VulkanTestReturnResult(vkBeginCommandBuffer(commandBuffer, &beginInfo),
                           "Failed to begin recording Vulkan command buffer!");

    const VkClearValue clearColor[2] = {
        {.color = {{0.0f, 0.64f, 0.91f, 1.0f}}},
        {.depthStencil = {1, 0}}
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
        2,
        clearColor
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

void DrawVertexBuffer(const VkCommandBuffer commandBuffer, const VkPipeline pipeline,
                             const VertexBuffer vertexBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, (VkBuffer[1]){vertexBuffer.buffer}, (VkDeviceSize[1]){0});

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &descriptorSets[currentFrame], 0, NULL);

    vkCmdDraw(commandBuffer, vertexBuffer.vertexCount, 1, 0, 0);
}

bool DrawRectInternal(const float ndcStartX, const float ndcStartY, const float ndcEndX, const float ndcEndY, const uint32_t color)
{
    GetColor(color);

    if (vertexBuffers.ui.vertexCount >= vertexBuffers.ui.maxVertices)
    {
        if (vertexBuffers.ui.vertexCount - vertexBuffers.ui.maxVertices >= vertexBuffers.ui.fallbackMaxVertices)
        {
            if (vertexBuffers.ui.fallbackMaxVertices)
            {
                vertexBuffers.ui.fallbackMaxVertices += 64;
                UiVertex *newVertices = realloc(vertexBuffers.ui.fallback,
                                                sizeof(UiVertex) * vertexBuffers.ui.fallbackMaxVertices);
                if (!newVertices)
                {
                    free(newVertices);
                    VulkanLogError("realloc of fallback UI vertex buffer failed!");
                    return false;
                }
            } else
            {
                vertexBuffers.ui.fallbackMaxVertices = vertexBuffers.ui.maxVertices + 64;
                vertexBuffers.ui.fallback = malloc(sizeof(UiVertex) * vertexBuffers.ui.fallbackMaxVertices);
                memcpy(vertexBuffers.ui.fallback, vertexBuffers.ui.vertices,
                       sizeof(UiVertex) * vertexBuffers.ui.maxVertices);
                if (!vertexBuffers.ui.fallback)
                {
                    VulkanLogError("malloc of fallback UI vertex buffer failed!");
                    return false;
                }
            }
        }
        vertexBuffers.ui.fallback[vertexBuffers.ui.vertexCount++] = (UiVertex){
            {ndcStartX, ndcStartY, 0, 0},
            {r, g, b, a},
            0
        };
        vertexBuffers.ui.fallback[vertexBuffers.ui.vertexCount++] = (UiVertex){
            {ndcEndX, ndcStartY, 0, 0},
            {r, g, b, a},
            0
        };
        vertexBuffers.ui.fallback[vertexBuffers.ui.vertexCount++] = (UiVertex){
            {ndcEndX, ndcEndY, 0, 0},
            {r, g, b, a},
            0
        };
        vertexBuffers.ui.fallback[vertexBuffers.ui.vertexCount++] = (UiVertex){
            {ndcStartX, ndcEndY, 0, 0},
            {r, g, b, a},
            0
        };

        return true;
    }

    vertexBuffers.ui.vertices[vertexBuffers.ui.vertexCount++] = (UiVertex){
        {ndcStartX, ndcStartY, 0, 0},
        {r, g, b, a},
        0
    };
    vertexBuffers.ui.vertices[vertexBuffers.ui.vertexCount++] = (UiVertex){{ndcEndX, ndcStartY, 0, 0}, {r, g, b, a}, 0};
    vertexBuffers.ui.vertices[vertexBuffers.ui.vertexCount++] = (UiVertex){{ndcEndX, ndcEndY, 0, 0}, {r, g, b, a}, 0};
    vertexBuffers.ui.vertices[vertexBuffers.ui.vertexCount++] = (UiVertex){{ndcStartX, ndcEndY, 0, 0}, {r, g, b, a}, 0};

    return true;
}
