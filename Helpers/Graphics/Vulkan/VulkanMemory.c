//
// Created by Noah on 12/18/2024.
//

#include "VulkanMemory.h"

bool AllocateMemory(MemoryInfo *memoryInfo, const uint32_t memoryTypeBits)
{
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & memoryInfo->type) == memoryInfo->type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memoryInfo->size,
				.memoryTypeIndex = i,
			};
			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &memoryInfo->memory),
					   "Failed to allocate buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to allocate buffer memory!\n");

	return true;
}

bool BindMemory(const Buffer *buffer)
{
	VulkanTest(vkBindBufferMemory(device,
								  buffer->buffer,
								  buffer->memoryAllocationInfo.memoryInfo->memory,
								  buffer->memoryAllocationInfo.offset),
			   "Failed to bind buffer memory!");

	return true;
}

inline bool CreateLocalMemory()
{
	if (!AllocateMemory(&memoryPools.localMemory, buffers.local.memoryAllocationInfo.memoryRequirements.memoryTypeBits))
	{
		return false;
	}
	if (!BindMemory(&buffers.local))
	{
		return false;
	}

	return true;
}

void SetSharedMemoryMappingInfo()
{
	buffers.walls.shadowVertexStaging = memoryPools.sharedMemory.mappedMemory +
										buffers.shared.memoryAllocationInfo.offset +
										buffers.walls.shadowStagingOffset;
	buffers.walls.shadowIndexStaging = memoryPools.sharedMemory.mappedMemory +
									   buffers.shared.memoryAllocationInfo.offset +
									   buffers.walls.shadowStagingOffset +
									   sizeof(ShadowVertex) * buffers.walls.shadowCount * 4;

	buffers.ui.vertexStaging = memoryPools.sharedMemory.mappedMemory +
							   buffers.shared.memoryAllocationInfo.offset +
							   buffers.ui.vertexStagingOffset;
	buffers.ui.indexStaging = memoryPools.sharedMemory.mappedMemory +
							  buffers.shared.memoryAllocationInfo.offset +
							  buffers.ui.indexStagingOffset;

	buffers.actors.instanceDataStaging = memoryPools.sharedMemory.mappedMemory +
										 buffers.shared.memoryAllocationInfo.offset +
										 buffers.actors.instanceDataStagingOffset;
	buffers.actors.drawInfoStaging = memoryPools.sharedMemory.mappedMemory +
									 buffers.shared.memoryAllocationInfo.offset +
									 buffers.actors.drawInfoStagingOffset;
	buffers.actors.walls.vertexStaging = memoryPools.sharedMemory.mappedMemory +
										 buffers.shared.memoryAllocationInfo.offset +
										 buffers.actors.walls.vertexStagingOffset;
	buffers.actors.walls.indexStaging = memoryPools.sharedMemory.mappedMemory +
										buffers.shared.memoryAllocationInfo.offset +
										buffers.actors.walls.indexStagingOffset;
}

bool MapSharedMemory()
{
	VulkanTest(vkMapMemory(device,
						   memoryPools.sharedMemory.memory,
						   0,
						   VK_WHOLE_SIZE,
						   0,
						   &memoryPools.sharedMemory.mappedMemory),
			   "Failed to map shared buffer memory!");

	SetSharedMemoryMappingInfo();

	return true;
}

inline bool CreateSharedMemory()
{
	if (!AllocateMemory(&memoryPools.sharedMemory,
						buffers.shared.memoryAllocationInfo.memoryRequirements.memoryTypeBits))
	{
		return false;
	}
	if (!BindMemory(&buffers.shared))
	{
		return false;
	}
	if (!MapSharedMemory())
	{
		return false;
	}

	return true;
}
