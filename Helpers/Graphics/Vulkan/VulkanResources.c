//
// Created by Noah on 12/18/2024.
//

#include "VulkanResources.h"

#include "VulkanMemory.h"

bool CreateLocalBuffer()
{
	buffers.walls.vertexSize = sizeof(WallVertex) * buffers.walls.maxWallCount * 4;
	buffers.walls.indexSize = sizeof(uint32_t) * buffers.walls.maxWallCount * 6;
	buffers.ui.vertexSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
	buffers.ui.indexSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;

	buffers.local.size = buffers.walls.vertexSize +
						 buffers.walls.indexSize +
						 buffers.ui.vertexSize +
						 buffers.ui.indexSize;
	buffers.local.memoryAllocationInfo.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
													VK_BUFFER_USAGE_TRANSFER_DST_BIT |
													VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
													VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	buffers.local.memoryAllocationInfo.memoryInfo = &memoryPools.localMemory;
	if (!CreateBuffer(&buffers.local, false))
	{
		return false;
	}

	return true;
}

void SetLocalBufferAliasingInfo()
{
	buffers.walls.bufferInfo = &buffers.local;
	buffers.walls.verticesOffset = 0;
	buffers.walls.indicesOffset = buffers.walls.vertexSize;

	buffers.ui.bufferInfo = &buffers.local;
	buffers.ui.verticesOffset = buffers.walls.vertexSize + buffers.walls.indexSize;
	buffers.ui.indicesOffset = buffers.walls.vertexSize + buffers.walls.indexSize + buffers.ui.vertexSize;
}

bool CreateSharedBuffer()
{
	buffers.ui.vertexStagingSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
	buffers.ui.indexStagingSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;
	const VkDeviceSize translationBufferSize = sizeof(mat4) * MAX_FRAMES_IN_FLIGHT;

	buffers.shared.size = buffers.ui.vertexStagingSize + buffers.ui.indexStagingSize + translationBufferSize;
	buffers.shared.memoryAllocationInfo.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
													 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	buffers.shared.memoryAllocationInfo.memoryInfo = &memoryPools.sharedMemory;
	if (!CreateBuffer(&buffers.shared, false))
	{
		return false;
	}

	if (!buffers.ui.vertices)
	{
		buffers.ui.vertices = malloc(sizeof(UiVertex) * buffers.ui.maxQuads * 4);
		if (!buffers.ui.vertices)
		{
			VulkanLogError("malloc of UI vertex buffer data pointer failed!\n");
			return false;
		}
	}
	if (!buffers.ui.indices)
	{
		buffers.ui.indices = malloc(sizeof(uint32_t) * buffers.ui.maxQuads * 6);
		if (!buffers.ui.indices)
		{
			VulkanLogError("malloc of UI index buffer data pointer failed!\n");

			return false;
		}
	}


	return true;
}

void SetSharedBufferAliasingInfo()
{
	const VkDeviceSize translationBufferSize = sizeof(mat4) * MAX_FRAMES_IN_FLIGHT;

	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		buffers.translation[i].bufferInfo = &buffers.shared;
		buffers.translation[i].offset = sizeof(mat4) * i;
	}
	buffers.ui.stagingBufferInfo = &buffers.shared;
	buffers.ui.verticesStagingOffset = translationBufferSize;
	buffers.ui.indicesStagingOffset = translationBufferSize + buffers.ui.vertexStagingSize;
}

// TODO: lossless
bool ResizeBuffer(Buffer *buffer, bool lossy, const MemoryMappingFunction MapMemory)
{
	vkDestroyBuffer(device, buffer->buffer, NULL);
	buffer->buffer = VK_NULL_HANDLE;

	vkFreeMemory(device, buffer->memoryAllocationInfo.memoryInfo->memory, NULL);
	buffer->memoryAllocationInfo.memoryInfo->memory = VK_NULL_HANDLE;

	if (!CreateBuffer(buffer, false))
	{
		return false;
	}

	// TODO: Fix to dynamically set aliasing information from buffer information
	if (buffer->memoryAllocationInfo.memoryInfo->type == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	{
		buffers.walls.vertexSize = sizeof(WallVertex) * buffers.walls.maxWallCount * 4;
		buffers.walls.indexSize = sizeof(uint32_t) * buffers.walls.maxWallCount * 6;
		buffers.ui.vertexSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
		buffers.ui.indexSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;
		SetLocalBufferAliasingInfo();
	} else if (buffer->memoryAllocationInfo.memoryInfo->type ==
			   (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		buffers.ui.vertexStagingSize = sizeof(UiVertex) * buffers.ui.maxQuads * 4;
		buffers.ui.indexStagingSize = sizeof(uint32_t) * buffers.ui.maxQuads * 6;
		SetSharedBufferAliasingInfo();
	}

	if (!AllocateMemory(buffer->memoryAllocationInfo.memoryInfo,
						buffer->memoryAllocationInfo.memoryRequirements.memoryTypeBits))
	{
		return false;
	}
	if (!BindMemory(buffer))
	{
		return false;
	}
	if (MapMemory)
	{
		return MapMemory();
	}

	return true;
}

// TODO: This function assumes the buffer (not sub-buffer, but the larger buffer) consumes the whole memory allocation
// TODO: Rewrite to allow for batched resizes
bool ResizeBufferRegion(Buffer *buffer,
						const VkDeviceSize offset,
						const VkDeviceSize oldSize,
						const VkDeviceSize newSize,
						const bool lossy,
						const MemoryMappingFunction MapMemory)
{
	const VkDeviceSize memorySize = buffer->memoryAllocationInfo.memoryInfo->size;
	void *otherBufferMemory = NULL;
	void *bufferMemory = NULL;
	Buffer stagingBuffer = {.buffer = VK_NULL_HANDLE};
	if (buffer->memoryAllocationInfo.memoryInfo->mappedMemory)
	{
		otherBufferMemory = malloc(memorySize - oldSize);

		memcpy(otherBufferMemory, buffer->memoryAllocationInfo.memoryInfo->mappedMemory, offset);
		memcpy(otherBufferMemory + offset,
			   buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset + oldSize,
			   memorySize - offset - oldSize);

		if (!lossy)
		{
			bufferMemory = malloc(oldSize);
			memcpy(bufferMemory, buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset, oldSize);
		}
	} else
	{
		MemoryInfo memoryInfo = {
			.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};
		stagingBuffer.memoryAllocationInfo = (MemoryAllocationInfo){
			.memoryInfo = &memoryInfo,
			.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		};
		stagingBuffer.size = memorySize - oldSize;
		if (!CreateBuffer(&stagingBuffer, true))
		{
			return false;
		}

		const VkBufferCopy regions[] = {
			{
				.size = offset,
			},
			{
				.srcOffset = offset + oldSize,
				.dstOffset = offset,
				.size = memorySize - offset - oldSize,
			},
		};
		if (!CopyBuffer(buffer->buffer, stagingBuffer.buffer, offset + oldSize == buffer->size ? 1 : 2, regions))
		{
			return false;
		}
	}

	buffer->size = memorySize - oldSize + newSize;
	if (!ResizeBuffer(buffer, true, MapMemory))
	{
		free(bufferMemory);
		free(otherBufferMemory);

		return false;
	}

	if (buffer->memoryAllocationInfo.memoryInfo->mappedMemory)
	{
		memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory, otherBufferMemory, offset);
		// ReSharper disable once CppDFANullDereference
		memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset + newSize,
			   otherBufferMemory + offset,
			   memorySize - offset - oldSize);
		if (!lossy)
		{
			memcpy(buffer->memoryAllocationInfo.memoryInfo->mappedMemory + offset, bufferMemory, oldSize);
		}

		free(bufferMemory);
		free(otherBufferMemory);
	} else
	{
		const VkBufferCopy regions[] = {
			{
				.size = offset,
			},
			{
				.srcOffset = offset,
				.dstOffset = offset + newSize,
				.size = memorySize - offset - oldSize,
			},
		};
		if (!CopyBuffer(stagingBuffer.buffer, buffer->buffer, offset + newSize == buffer->size ? 1 : 2, regions))
		{
			return false;
		}

		if (!DestroyBuffer(&stagingBuffer))
		{
			return false;
		}
	}

	return true;
}

void UpdateDescriptorSets()
{
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo uniformBufferInfo = {
			.buffer = buffers.translation[i].bufferInfo->buffer,
			.offset = buffers.translation[i].offset,
			.range = sizeof(mat4),
		};

		VkDescriptorImageInfo imageInfo[TEXTURE_ASSET_COUNT];
		for (uint16_t textureIndex = 0; textureIndex < TEXTURE_ASSET_COUNT; textureIndex++)
		{
			imageInfo[textureIndex] = (VkDescriptorImageInfo){
				.sampler = textureSamplers.nearestRepeat,
				.imageView = texturesImageView[textureIndex],
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
		}

		const VkWriteDescriptorSet writeDescriptorList[2] = {
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = NULL,
				.dstSet = descriptorSets[i],
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = NULL,
				.pBufferInfo = &uniformBufferInfo,
				.pTexelBufferView = NULL,
			},
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = NULL,
				.dstSet = descriptorSets[i],
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = TEXTURE_ASSET_COUNT,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = imageInfo,
				.pBufferInfo = NULL,
				.pTexelBufferView = NULL,
			},
		};
		vkUpdateDescriptorSets(device, 2, writeDescriptorList, 0, NULL);
	}
}
