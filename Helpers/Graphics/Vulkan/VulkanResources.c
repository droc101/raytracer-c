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
