//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"
#include "../../../Assets/AssetReader.h"
#include "../../../Structs/GlobalState.h"
#include "../../Core/DataReader.h"

bool VK_Init(SDL_Window *window)
{
    vk_window = window;
    if (CreateInstance() && CreateSurface() && PickPhysicalDevice() && CreateLogicalDevice() && CreateSwapChain() &&
        CreateImageViews() && CreateRenderPass() && CreateDescriptorSetLayouts() && CreateGraphicsPipelineCache() &&
        CreateGraphicsPipelines() && CreateCommandPools() && CreateColorImage() && CreateDepthImage() &&
        CreateFramebuffers() && LoadTextures() && CreateTexturesImageView() && CreateTextureSampler() &&
        CreateVertexBuffers() && CreateUniformBuffers() && CreateDescriptorPool() && CreateDescriptorSets() &&
        CreateCommandBuffers() && CreateSyncObjects())
    {
        const DataUniformBufferObject dataBufferObject = {
            texturesAssetIDMap[ReadUintA(DecompressAsset(gztex_actor_iq), 12)]
        };

        memcpy(buffers.data.data, &dataBufferObject, sizeof(dataBufferObject));

        return true;
    }
    VK_Cleanup();

    return false;
}

VkResult VK_FrameStart()
{
    if (minimized) return VK_NOT_READY;

    VulkanTestReturnResult(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX),
                           "Failed to wait for Vulkan fences!");

    VulkanTestReturnResult(vkResetFences(device, 1, &inFlightFences[currentFrame]), "Failed to reset Vulkan fences!");

    const VkResult acquireNextImageResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                                                                  imageAvailableSemaphores[currentFrame],
                                                                  VK_NULL_HANDLE, &swapchainImageIndex);

    if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireNextImageResult == VK_SUBOPTIMAL_KHR)
    {
        if (RecreateSwapChain()) return acquireNextImageResult;
    }
    VulkanTestReturnResult(acquireNextImageResult, "Failed to acquire next Vulkan image index!");

    VulkanTestReturnResult(vkResetCommandBuffer(commandBuffers[currentFrame], 0),
                           "Failed to reset Vulkan command buffer!");

    buffers.ui.vertexCount = 0;

    return VK_SUCCESS;
}

VkResult VK_FrameEnd()
{
    VulkanTestReturnResult(BeginRenderPass(commandBuffers[currentFrame], swapchainImageIndex),
                           "Failed to begin render pass!");

    const GlobalState *g = GetState();
    if (g->currentState == MAIN_STATE || g->currentState == PAUSE_STATE)
    {
        DrawVertexBuffer(commandBuffers[currentFrame], pipelines.walls, buffers.walls);
    }

    vkCmdNextSubpass(commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);

    if (buffers.ui.fallbackMaxVertices > 0)
    {
        VulkanLogError("UI Buffer Resized!!\n"); // TODO remove me when buffer size figured out

        vkUnmapMemory(device, memoryPools.sharedMemory.memory);
        vkDestroyBuffer(device, buffers.ui.buffer, NULL);
        vkFreeMemory(device, memoryPools.sharedMemory.memory, NULL);

        buffers.ui.memoryAllocationInfo.memoryInfo = &memoryPools.sharedMemory;
        CreateBuffer(&buffers.ui.buffer, sizeof(UiVertex) * buffers.ui.fallbackMaxVertices,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false,
                     &buffers.ui.memoryAllocationInfo);

        // TODO Resizing the buffers does not work
        VulkanTest(vkMapMemory(device, memoryPools.sharedMemory.memory, 0,
                       sizeof(*buffers.ui.vertices) * buffers.ui.fallbackMaxVertices, 0,
                       (void**)&buffers.ui.vertices), "Failed to map ui vertex buffer memory!");

        memcpy(buffers.ui.vertices, buffers.ui.fallback,
               sizeof(UiVertex) * buffers.ui.fallbackMaxVertices);

        buffers.ui.maxVertices = buffers.ui.fallbackMaxVertices;
        buffers.ui.fallbackMaxVertices = 0;
        free(buffers.ui.fallback);
    }

    DrawVertexBuffer(commandBuffers[currentFrame], pipelines.ui, buffers.ui);

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
        if (RecreateSwapChain()) return queuePresentResult;
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

        vkUnmapMemory(device, memoryPools.sharedMemory.memory);

        CleanupSwapChain();

        vkDestroySampler(device, textureSamplers.linearRepeat, NULL);
        vkDestroySampler(device, textureSamplers.nearestRepeat, NULL);
        vkDestroySampler(device, textureSamplers.linearNoRepeat, NULL);
        vkDestroySampler(device, textureSamplers.nearestNoRepeat, NULL);
        for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
        {
            vkDestroyImageView(device, texturesImageView[textureIndex], NULL);
            vkDestroyImage(device, textures[textureIndex].image, NULL);
        }
        vkFreeMemory(device, textureMemory, NULL);

        CleanupColorImage();
        CleanupDepthImage();

        vkDestroyPipelineCache(device, pipelineCache, NULL);
        CleanupPipeline();

        vkDestroyRenderPass(device, renderPass, NULL);

        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);

        vkDestroyBuffer(device, buffers.ui.buffer, NULL);
        vkDestroyBuffer(device, buffers.data.buffer, NULL);
        vkDestroyBuffer(device, buffers.walls.buffer, NULL);
        for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(device, buffers.translation[i].buffer, NULL);
        }

        vkFreeMemory(device, memoryPools.localMemory.memory, NULL);
        vkFreeMemory(device, memoryPools.sharedMemory.memory, NULL);

        CleanupSyncObjects();

        vkDestroyCommandPool(device, graphicsCommandPool, NULL);
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
    return physicalDevice.properties.limits.framebufferColorSampleCounts & physicalDevice.properties.limits.framebufferDepthSampleCounts;
}

bool VK_DrawColoredQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t color)
{
    return DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), 0, 0, 0, 0, color,
                            -1);
}

bool VK_DrawColoredQuadsBatched(const float *vertices, const int32_t quadCount, const uint32_t color)
{
    for (int i = 0; i < quadCount; i++)
    {
        const uint32_t index = i * 8;
        if (!DrawQuadInternal((mat4){
                                  {vertices[index + 0], vertices[index + 1], 0, 0},
                                  {vertices[index + 2], vertices[index + 3], 0, 0},
                                  {vertices[index + 4], vertices[index + 5], 0, 0},
                                  {vertices[index + 6], vertices[index + 7], 0, 0}
                              }, color, -1))
        {
            return false;
        }
    }

    return true;
}

bool VK_DrawTexturedQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint8_t *texture)
{
    return DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), 0, 0, 1, 1,
                            0xFFFFFFFF, TextureIndex(texture));
}

bool VK_DrawTexturedQuadMod(const int32_t x,
                            const int32_t y,
                            const int32_t w,
                            const int32_t h,
                            const uint8_t *texture,
                            const uint32_t color)
{
    return DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), 0, 0, 1, 1, color,
                            TextureIndex(texture));
}

bool VK_DrawTexturedQuadRegion(const int32_t x,
                               const int32_t y,
                               const int32_t w,
                               const int32_t h,
                               const int32_t regionX,
                               const int32_t regionY,
                               const int32_t regionW,
                               const int32_t regionH,
                               const uint8_t *texture)
{
    const uint8_t *decompressed = DecompressAsset(texture);

    const uint32_t width = ReadUintA(decompressed, 4);
    const uint32_t height = ReadUintA(decompressed, 8);

    const float startU = (float)regionX / (float)width;
    const float startV = (float)regionY / (float)height;

    return DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), startU, startV,
                            startU + (float)regionW / (float)width, startV + (float)regionH / (float)height, 0xFFFFFFFF,
                            texturesAssetIDMap[ReadUintA(decompressed, 12)]);
}

bool VK_DrawTexturedQuadRegionMod(const int32_t x,
                                  const int32_t y,
                                  const int32_t w,
                                  const int32_t h,
                                  const int32_t regionX,
                                  const int32_t regionY,
                                  const int32_t regionW,
                                  const int32_t regionH,
                                  const uint8_t *texture,
                                  const uint32_t color)
{
    const uint8_t *decompressed = DecompressAsset(texture);

    const uint32_t width = ReadUintA(decompressed, 4);
    const uint32_t height = ReadUintA(decompressed, 8);

    const float startU = (float)regionX / (float)width;
    const float startV = (float)regionY / (float)height;

    return DrawRectInternal(VK_X_TO_NDC(x), VK_Y_TO_NDC(y), VK_X_TO_NDC(x + w), VK_Y_TO_NDC(y + h), startU, startV,
                            startU + (float)regionW / (float)width, startV + (float)regionH / (float)height, color,
                            texturesAssetIDMap[ReadUintA(decompressed, 12)]);
}

bool VK_DrawTexturedQuadsBatched(const float *vertices,
                                 const int quadCount,
                                 const uint8_t *texture,
                                 const uint32_t color)
{
    for (int i = 0; i < quadCount; i++)
    {
        const uint32_t index = i * 16;
        if (!DrawQuadInternal((mat4){
                                  {vertices[index + 0], vertices[index + 1], vertices[index + 2], vertices[index + 3]},
                                  {vertices[index + 4], vertices[index + 5], vertices[index + 6], vertices[index + 7]},
                                  {
                                      vertices[index + 8],
                                      vertices[index + 9],
                                      vertices[index + 10],
                                      vertices[index + 11]
                                  },
                                  {
                                      vertices[index + 12],
                                      vertices[index + 13],
                                      vertices[index + 14],
                                      vertices[index + 15]
                                  }
                              }, color, TextureIndex(texture)))
        {
            return false;
        }
    }

    return true;
}

bool VK_DrawLine(const int32_t startX,
                 const int32_t startY,
                 const int32_t endX,
                 const int32_t endY,
                 const float thickness,
                 const uint32_t color)
{
    const float dx = (float)endX - (float)startX;
    const float dy = (float)endY - (float)startY;
    const float distance = sqrtf(dx * dx + dy * dy);

    if (thickness == 1)
    {
        const mat4 matrix = {
            {VK_X_TO_NDC(-dy / distance + (float)startX), VK_Y_TO_NDC(dx / distance + (float)startY), 0, 0},
            {VK_X_TO_NDC(-dy / distance + (float)endX), VK_Y_TO_NDC(dx / distance + (float)endY), 0, 0},
            {VK_X_TO_NDC(endX), VK_Y_TO_NDC(endY), 0, 0},
            {VK_X_TO_NDC(startX), VK_Y_TO_NDC(startY), 0, 0}
        };

        return DrawQuadInternal(matrix, color, -1);
    }

    const float size = thickness / 2;

    const mat4 matrix = {
        {VK_X_TO_NDC(-size * dy / distance + (float)startX), VK_Y_TO_NDC(size * dx / distance + (float)startY), 0, 0},
        {VK_X_TO_NDC(-size * dy / distance + (float)endX), VK_Y_TO_NDC(size * dx / distance + (float)endY), 0, 0},
        {VK_X_TO_NDC(size * dy / distance + (float)endX), VK_Y_TO_NDC(-size * dx / distance + (float)endY), 0, 0},
        {VK_X_TO_NDC(size * dy / distance + (float)startX), VK_Y_TO_NDC(-size * dx / distance + (float)startY), 0, 0}
    };

    return DrawQuadInternal(matrix, color, -1);
}

bool VK_DrawRectOutline(const int32_t x,
                        const int32_t y,
                        const int32_t w,
                        const int32_t h,
                        const float thickness,
                        const uint32_t color)
{
    VK_DrawLine(x, y, x + w, y, thickness, color);
    VK_DrawLine(x + w, y, x + w, y + h, thickness, color);
    VK_DrawLine(x + w, y + h, x, y + h, thickness, color);
    VK_DrawLine(x, y + h, x, y, thickness, color);

    return true;
}

void VK_ClearColor(const uint32_t color)
{
    GET_COLOR(color);

    clearColor = (VkClearColorValue){{r, g, b, a}};
    VK_ClearScreen();
}

void VK_ClearScreen() {}

void VK_ClearDepthOnly() {}

void VK_SetTexParams(const uint8_t *texture, const bool linear, const bool repeat)
{
    const uint32_t textureIndex = TextureIndex(texture);
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorImageInfo imageInfo = {
            textureSamplers.nearestNoRepeat,
            texturesImageView[textureIndex],
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        if (linear && repeat)
        {
            imageInfo.sampler = textureSamplers.linearRepeat;
        } else if (linear)
        {
            imageInfo.sampler = textureSamplers.linearNoRepeat;
        } else if (repeat)
        {
            imageInfo.sampler = textureSamplers.nearestRepeat;
        }

        const VkWriteDescriptorSet writeDescriptor = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            descriptorSets[i],
            1,
            textureIndex,
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            &imageInfo,
            NULL,
            NULL
        };
        vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, NULL);
    }
}
