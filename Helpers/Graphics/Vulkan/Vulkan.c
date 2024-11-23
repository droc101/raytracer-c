//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"
#include "../Drawing.h"
#include "../../../Assets/AssetReader.h"
#include "../../Core/DataReader.h"

bool VK_Init(SDL_Window *window)
{
    vk_window = window;
    if (CreateInstance() && CreateSurface() && PickPhysicalDevice() && CreateLogicalDevice() && CreateSwapChain() &&
        CreateImageViews() && CreateRenderPass() && CreateDescriptorSetLayouts() && CreateGraphicsPipelines() &&
        CreateCommandPools() && CreateColorImage() && CreateDepthImage() && CreateFramebuffers() && LoadTextures() &&
        CreateTexturesImageView() && CreateTextureSampler() && CreateVertexBuffers() && CreateUniformBuffers() &&
        CreateDescriptorPool() && CreateDescriptorSets() && CreateCommandBuffers() && CreateSyncObjects())
    {
        const DataBufferObject dataBufferObject = {
            texturesAssetIDMap[ReadUintA(DecompressAsset(gztex_actor_iq), 12)]
        };

        memcpy(mappedDataBuffer, &dataBufferObject, sizeof(dataBufferObject));

        vertexBuffers.ui.vertices = calloc(vertexBuffers.ui.maxVertices, sizeof(UiVertex));

        VulkanTest(
            vkMapMemory(device, vertexBuffers.sharedMemory, 0, sizeof(*vertexBuffers.ui.vertices) * vertexBuffers.ui.
                maxVertices, 0, (void**)&vertexBuffers.ui.vertices),
            "Failed to map ui vertex buffer memory!");

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

    vertexBuffers.ui.vertexCount = 0;

    return VK_SUCCESS;
}

VkResult VK_FrameEnd()
{
    VulkanTestReturnResult(BeginRenderPass(commandBuffers[currentFrame], swapchainImageIndex),
                           "Failed to begin render pass!");

    DrawVertexBuffer(commandBuffers[currentFrame], pipelines.walls, vertexBuffers.walls);

    vkCmdNextSubpass(commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);

    if (vertexBuffers.ui.fallbackMaxVertices > 0)
    {
        VulkanLogError("UI Buffer Resized!!"); // TODO remove me when buffer size figured out

        vkUnmapMemory(device, vertexBuffers.sharedMemory);
        vkDestroyBuffer(device, vertexBuffers.ui.buffer, NULL);
        vkFreeMemory(device, vertexBuffers.sharedMemory, NULL);

        CreateBuffer(sizeof(UiVertex) * vertexBuffers.ui.fallbackMaxVertices,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &vertexBuffers.ui.buffer, &vertexBuffers.sharedMemory);

        vertexBuffers.ui.vertices = malloc(sizeof(UiVertex) * vertexBuffers.ui.fallbackMaxVertices);
        if (!vertexBuffers.ui.vertices)
        {
            VulkanLogError("malloc of UI vertex buffer failed!");
            return false;
        }
        VulkanTest(vkMapMemory(device, vertexBuffers.sharedMemory, 0,
                       sizeof(*vertexBuffers.ui.vertices) * vertexBuffers.ui.fallbackMaxVertices, 0,
                       (void**)&vertexBuffers.ui.vertices), "Failed to map ui vertex buffer memory!");

        memcpy(&vertexBuffers.ui.vertices[vertexBuffers.ui.maxVertices], vertexBuffers.ui.fallback,
               sizeof(UiVertex) * vertexBuffers.ui.fallbackMaxVertices);

        vertexBuffers.ui.maxVertices = vertexBuffers.ui.fallbackMaxVertices;
        vertexBuffers.ui.fallbackMaxVertices = 0;
        free(vertexBuffers.ui.fallback);
    }

    DrawVertexBuffer(commandBuffers[currentFrame], pipelines.ui, vertexBuffers.ui);

    VulkanTestReturnResult(EndRenderPass(commandBuffers[currentFrame]), "Failed to end render pass!");

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
    free(vertexBuffers.ui.vertices);

    if (device)
    {
        VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for Vulkan device to become idle!");

        vkUnmapMemory(device, vertexBuffers.sharedMemory);

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

        CleanupPipeline();

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

        vkDestroyBuffer(device, vertexBuffers.walls.buffer, NULL);
        vkFreeMemory(device, vertexBuffers.localMemory, NULL);

        vkDestroyBuffer(device, vertexBuffers.ui.buffer, NULL);
        vkFreeMemory(device, vertexBuffers.sharedMemory, NULL);

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

inline VkSampleCountFlags VK_GetSampleCount()
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    return properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
}

bool VK_DrawRect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t color)
{
    GetColor(color);
    const float ndcStartX = VK_X_TO_NDC(x);
    const float ndcStartY = VK_Y_TO_NDC(y);
    const float ndcEndX = VK_X_TO_NDC(x + w);
    const float ndcEndY = VK_Y_TO_NDC(y + h);

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
