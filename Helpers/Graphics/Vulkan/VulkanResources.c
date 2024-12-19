//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"
#include "VulkanHelpers.h"

bool CreateLocalBuffer()
{
    const VkDeviceSize size = sizeof(WallVertex) * buffers.walls.maxWallCount * 4 +
                              sizeof(uint32_t) * buffers.walls.maxWallCount * 6;
    const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    buffers.local.memoryAllocationInfo.memoryInfo = &memoryPools.localMemory;
    if (!CreateBuffer(&buffers.local.buffer, size, usageFlags, false, &buffers.local.memoryAllocationInfo))
    {
        return false;
    }

    return true;
}

bool SetLocalBufferAliasingInfo()
{
    buffers.walls.bufferInfo = &buffers.local;
    buffers.walls.verticesOffset = 0;
    buffers.walls.indicesOffset = 4 * buffers.walls.maxWallCount * sizeof(WallVertex);

    return true;
}

bool CreateSharedBuffer()
{
    const VkDeviceSize size = sizeof(mat4) * MAX_FRAMES_IN_FLIGHT +
                              sizeof(UiVertex) * buffers.ui.maxQuads * 4 +
                              sizeof(uint32_t) * buffers.ui.maxQuads * 6;
    const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    buffers.shared.memoryAllocationInfo.memoryInfo = &memoryPools.sharedMemory;
    if (!CreateBuffer(&buffers.shared.buffer, size, usageFlags, false, &buffers.shared.memoryAllocationInfo))
    {
        return false;
    }

    return true;
}

bool SetSharedBufferAliasingInfo()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        buffers.translation[i].bufferInfo = &buffers.shared;
        buffers.translation[i].offset = sizeof(mat4) * i;
    }
    buffers.ui.bufferInfo = &buffers.shared;
    buffers.ui.verticesOffset = sizeof(mat4) * MAX_FRAMES_IN_FLIGHT;
    buffers.ui.indicesOffset = sizeof(mat4) * MAX_FRAMES_IN_FLIGHT + sizeof(UiVertex) * buffers.ui.maxQuads * 4;

    return true;
}

void UpdateDescriptorSets()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo uniformBufferInfo = {
            buffers.translation[i].bufferInfo->buffer,
            buffers.translation[i].offset,
            sizeof(mat4)
        };

        VkDescriptorImageInfo imageInfo[TEXTURE_ASSET_COUNT];
        for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
        {
            imageInfo[textureIndex] = (VkDescriptorImageInfo){
                textureSamplers.nearestRepeat,
                texturesImageView[textureIndex],
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }

        const VkWriteDescriptorSet writeDescriptorList[2] = {
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
            }
        };
        vkUpdateDescriptorSets(device, 2, writeDescriptorList, 0, NULL);
    }
}
