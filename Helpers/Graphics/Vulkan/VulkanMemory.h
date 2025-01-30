//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANMEMORY_H
#define VULKANMEMORY_H

#include <stdbool.h>
#include "VulkanHelpers.h"

/// memoryTypeBits is a bitmask where bit n is set if the nth memory type of the
/// @c VkPhysicalDeviceMemoryProperties struct for the physical device is a supported memory type for the resource.
bool AllocateMemory(MemoryInfo *memoryInfo, uint32_t memoryTypeBits);

bool BindMemory(const Buffer *buffer);

bool CreateLocalMemory();

void SetSharedMemoryMappingInfo();

bool MapSharedMemory();

bool CreateSharedMemory();

#endif //VULKANMEMORY_H
