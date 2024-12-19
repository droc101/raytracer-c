//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"
#include "VulkanHelpers.h"

bool CreateLocalBuffer()
{
    const VkDeviceSize size = buffers.walls.maxWallCount * (2 * sizeof(WallVertex) + sizeof(WallInfo));
    const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

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
    buffers.walls.offsets[0] = 0;
    buffers.walls.offsets[1] = 2 * buffers.walls.maxWallCount * sizeof(WallVertex);

    return true;
}

bool CreateSharedBuffer()
{
    const VkDeviceSize size = sizeof(UiVertex) * buffers.ui.maxVertices + sizeof(mat4) * MAX_FRAMES_IN_FLIGHT;
    const VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
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
    buffers.ui.bufferInfo = &buffers.shared;
    buffers.ui.offset = 0;
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        buffers.translation[i].bufferInfo = &buffers.shared;
        buffers.translation[i].offset = sizeof(UiVertex) * buffers.ui.maxVertices + sizeof(mat4) * i;
    }

    return true;
}
