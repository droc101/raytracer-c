//
// Created by Noah on 12/18/2024.
//

#include "VulkanMemory.h"
#include "VulkanHelpers.h"

bool AllocateLocalMemory()
{
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryPools.localMemory.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & memoryPools.localMemory.type) ==
					memoryPools.localMemory.type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memoryPools.localMemory.size,
				.memoryTypeIndex = i,
			};
			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &memoryPools.localMemory.memory),
					   "Failed to allocate device local buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to allocate device local buffer memory!");

	return false;
}

inline bool BindLocalMemory()
{
	VulkanTest(vkBindBufferMemory(device,
								  buffers.local.buffer,
								  memoryPools.localMemory.memory,
								  buffers.local.memoryAllocationInfo.offset),
			   "Failed to bind local buffer memory!");

	return true;
}

inline bool CreateLocalMemory()
{
	AllocateLocalMemory();
	BindLocalMemory();

	return true;
}

bool AllocateSharedMemory()
{
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryPools.sharedMemory.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & memoryPools.sharedMemory.type) ==
					memoryPools.sharedMemory.type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memoryPools.sharedMemory.size,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &memoryPools.sharedMemory.memory),
					   "Failed to allocate device shared buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to allocate device shared buffer memory!");

	return false;
}

inline bool BindSharedMemory()
{
	VulkanTest(vkBindBufferMemory(device,
								  buffers.shared.buffer,
								  memoryPools.sharedMemory.memory,
								  buffers.shared.memoryAllocationInfo.offset),
			   "Failed to bind shared buffer memory!");

	return true;
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

	buffers.ui.vertices = memoryPools.sharedMemory.mappedMemory +
						  buffers.shared.memoryAllocationInfo.offset +
						  buffers.ui.verticesOffset;
	buffers.ui.indices = memoryPools.sharedMemory.mappedMemory +
						 buffers.shared.memoryAllocationInfo.offset +
						 buffers.ui.indicesOffset;
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		buffers.translation[i].data = memoryPools.sharedMemory.mappedMemory +
									  buffers.shared.memoryAllocationInfo.offset +
									  buffers.translation[i].offset;
	}

	return true;
}

inline bool CreateSharedMemory()
{
	AllocateSharedMemory();
	BindSharedMemory();
	MapSharedMemory();

	return true;
}
