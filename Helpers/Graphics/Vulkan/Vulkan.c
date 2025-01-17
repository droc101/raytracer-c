//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "../../../Structs/GlobalState.h"
#include "../../CommonAssets.h"
#include "VulkanHelpers.h"
#include "VulkanInternal.h"
#include "VulkanMemory.h"
#include "VulkanResources.h"

const Level *loadedLevel = NULL;

bool VK_Init(SDL_Window *window)
{
	vk_window = window;
	// clang-format off
	if (CreateInstance() && CreateSurface() && PickPhysicalDevice() && CreateLogicalDevice() && CreateSwapChain() &&
		CreateImageViews() && CreateRenderPass() && CreateDescriptorSetLayouts() && CreateGraphicsPipelineCache() &&
		CreateGraphicsPipelines() && CreateCommandPools() && CreateColorImage() && CreateDepthImage() &&
		CreateFramebuffers() && InitTextures() && CreateTexturesImageView() && CreateTextureSampler() &&
		CreateBuffers() && AllocateMemoryPools() && CreateDescriptorPool() && CreateDescriptorSets() &&
		CreateCommandBuffers() && CreateSyncObjects())
	{
		return true;
	}
	// clang-format on

	return VK_Cleanup();
}

VkResult VK_FrameStart()
{
	if (minimized)
	{
		return VK_NOT_READY;
	}

	VulkanTestReturnResult(vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX),
						   "Failed to wait for Vulkan fences!");

	VulkanTestReturnResult(vkResetFences(device, 1, &inFlightFences[currentFrame]), "Failed to reset Vulkan fences!");

	const VkResult acquireNextImageResult = vkAcquireNextImageKHR(device,
																  swapChain,
																  UINT64_MAX,
																  imageAvailableSemaphores[currentFrame],
																  VK_NULL_HANDLE,
																  &swapchainImageIndex);

	if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireNextImageResult == VK_SUBOPTIMAL_KHR)
	{
		if (RecreateSwapChain())
		{
			return acquireNextImageResult;
		}
	}
	VulkanTestReturnResult(acquireNextImageResult, "Failed to acquire next Vulkan image index!");

	VulkanTestReturnResult(vkResetCommandBuffer(commandBuffers[currentFrame], 0),
						   "Failed to reset Vulkan command buffer!");

	buffers.ui.quadCount = 0;

	return VK_SUCCESS;
}

VkResult VK_FrameEnd()
{
	if (buffers.ui.shouldResize)
	{
		if (currentFrame == 0)
		{
			VulkanTestReturnResult(vkWaitForFences(device,
												   1,
												   &inFlightFences[MAX_FRAMES_IN_FLIGHT - 1],
												   VK_TRUE,
												   UINT64_MAX),
								   "Failed to wait for Vulkan fences!");
		} else
		{
			VulkanTestReturnResult(vkWaitForFences(device, 1, &inFlightFences[currentFrame - 1], VK_TRUE, UINT64_MAX),
								   "Failed to wait for Vulkan fences!");
		}

		if (buffers.ui.verticesStagingOffset <= buffers.ui.indicesStagingOffset &&
			buffers.ui.indicesStagingOffset == buffers.ui.verticesStagingOffset + buffers.ui.vertexStagingSize)
		{
			if (!ResizeBufferRegion(&buffers.shared,
									buffers.ui.verticesStagingOffset,
									buffers.ui.vertexStagingSize + buffers.ui.indexStagingSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4 +
											sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									MapSharedMemory))
			{
				return VK_ERROR_UNKNOWN;
			}
		} else if (buffers.ui.indicesStagingOffset < buffers.ui.verticesStagingOffset &&
				   buffers.ui.verticesStagingOffset == buffers.ui.indicesStagingOffset + buffers.ui.indexStagingSize)
		{
			if (!ResizeBufferRegion(&buffers.shared,
									buffers.ui.indicesStagingOffset,
									buffers.ui.indexStagingSize + buffers.ui.vertexStagingSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4 +
											sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									MapSharedMemory))
			{
				return VK_ERROR_UNKNOWN;
			}
		} else
		{
			if (!ResizeBufferRegion(&buffers.shared,
									buffers.ui.verticesStagingOffset,
									buffers.ui.vertexStagingSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4,
									true,
									MapSharedMemory))
			{
				return VK_ERROR_UNKNOWN;
			}
			if (!ResizeBufferRegion(&buffers.shared,
									buffers.ui.indicesStagingOffset,
									buffers.ui.indexStagingSize,
									sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									MapSharedMemory))
			{
				return VK_ERROR_UNKNOWN;
			}
		}


		if (buffers.ui.verticesOffset <= buffers.ui.indicesOffset &&
			buffers.ui.indicesOffset == buffers.ui.verticesOffset + buffers.ui.vertexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.ui.verticesOffset,
									buffers.ui.vertexSize + buffers.ui.indexSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4 +
											sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									NULL))
			{
				return VK_ERROR_UNKNOWN;
			}
		} else if (buffers.ui.indicesOffset < buffers.ui.verticesOffset &&
				   buffers.ui.verticesOffset == buffers.ui.indicesOffset + buffers.ui.indexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.ui.indicesOffset,
									buffers.ui.indexSize + buffers.ui.vertexSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4 +
											sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									NULL))
			{
				return VK_ERROR_UNKNOWN;
			}
		} else
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.ui.verticesOffset,
									buffers.ui.vertexSize,
									sizeof(UiVertex) * buffers.ui.maxQuads * 4,
									true,
									NULL))
			{
				return VK_ERROR_UNKNOWN;
			}
			if (!ResizeBufferRegion(&buffers.local,
									buffers.ui.indicesOffset,
									buffers.ui.indexSize,
									sizeof(uint32_t) * buffers.ui.maxQuads * 6,
									true,
									NULL))
			{
				return VK_ERROR_UNKNOWN;
			}
		}

		UpdateUniformBufferDescriptorSets();

		buffers.ui.shouldResize = false;
	}
	memcpy(buffers.ui.vertexStaging, buffers.ui.vertices, sizeof(UiVertex) * buffers.ui.quadCount * 4);
	memcpy(buffers.ui.indexStaging, buffers.ui.indices, sizeof(uint32_t) * buffers.ui.quadCount * 6);

	if (buffers.ui.quadCount > 0)
	{
		const VkCommandBuffer commandBuffer;
		if (!BeginCommandBuffer(&commandBuffer, transferCommandPool))
		{
			return false;
		}

		vkCmdCopyBuffer(commandBuffer,
						buffers.ui.stagingBufferInfo->buffer,
						buffers.ui.bufferInfo->buffer,
						2,
						(VkBufferCopy[]){
							{
								.srcOffset = buffers.ui.verticesStagingOffset,
								.dstOffset = buffers.ui.verticesOffset,
								.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
							},
							{
								.srcOffset = buffers.ui.indicesStagingOffset,
								.dstOffset = buffers.ui.indicesOffset,
								.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
							},
						});

		if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue))
		{
			return false;
		}
	}

	VulkanTestReturnResult(BeginRenderPass(commandBuffers[currentFrame], swapchainImageIndex),
						   "Failed to begin render pass!");

	const GlobalState *g = GetState();
	if (g->currentState == MAIN_STATE || g->currentState == PAUSE_STATE && buffers.walls.wallCount > 0)
	{
		const uint32_t skyColor = (loadedLevel->skyColor & 0xFFFFFF00) | (TextureIndex(TEXTURE("level_sky")) & 0xFF);
		vkCmdPushConstants(commandBuffers[currentFrame],
						   pipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT,
						   0,
						   8,
						   (vec2){(float)loadedLevel->player.pos.x, (float)loadedLevel->player.pos.y});
		vkCmdPushConstants(commandBuffers[currentFrame],
						   pipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT,
						   8,
						   4,
						   &skyModel->packedVertsUvsCount);
		vkCmdPushConstants(commandBuffers[currentFrame],
						   pipelineLayout,
						   VK_SHADER_STAGE_FRAGMENT_BIT,
						   12,
						   4,
						   &skyColor);

		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.walls);

		vkCmdBindVertexBuffers(commandBuffers[currentFrame],
							   0,
							   1,
							   &buffers.walls.bufferInfo->buffer,
							   &buffers.walls.verticesOffset);

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.walls.bufferInfo->buffer,
							 buffers.walls.indicesOffset,
							 VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[currentFrame],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayout,
								0,
								1,
								&descriptorSets[currentFrame],
								0,
								NULL);

		vkCmdDrawIndexed(commandBuffers[currentFrame],
						 buffers.walls.wallCount * 6 + buffers.walls.skyIndexCount,
						 1,
						 0,
						 0,
						 0);
	}

	vkCmdNextSubpass(commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);

	if (buffers.ui.quadCount > 0)
	{
		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.ui);

		vkCmdBindVertexBuffers(commandBuffers[currentFrame],
							   0,
							   1,
							   &buffers.ui.bufferInfo->buffer,
							   &buffers.ui.verticesOffset);

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.ui.bufferInfo->buffer,
							 buffers.ui.indicesOffset,
							 VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[currentFrame],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayout,
								0,
								1,
								&descriptorSets[currentFrame],
								0,
								NULL);

		vkCmdDrawIndexed(commandBuffers[currentFrame], buffers.ui.quadCount * 6, 1, 0, 0, 0);
	}

	VulkanTestReturnResult(EndRenderPass(commandBuffers[currentFrame]), "Failed to end render pass!");

	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &imageAvailableSemaphores[currentFrame],
		.pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[currentFrame],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &renderFinishedSemaphores[currentFrame],
	};

	VulkanTestReturnResult(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]),
						   "Failed to submit Vulkan draw command buffer!");

	const VkSwapchainKHR swapChains[] = {swapChain};
	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
		.swapchainCount = 1,
		.pSwapchains = swapChains,
		.pImageIndices = &swapchainImageIndex,
		.pResults = NULL,
	};

	const VkResult queuePresentResult = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR)
	{
		if (RecreateSwapChain())
		{
			return queuePresentResult;
		}
	}
	VulkanTestReturnResult(queuePresentResult, "Failed to queue frame for presentation!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return VK_SUCCESS;
}

VkResult VK_RenderLevel(const Level *level, const Camera *camera)
{
	if (loadedLevel != level)
	{
		if (!VK_LoadLevelWalls(level))
		{
			return VK_ERROR_UNKNOWN;
		}
	}
	UpdateUniformBuffer(camera, currentFrame);
	return VK_SUCCESS;
}

bool VK_Cleanup()
{
	if (device)
	{
		VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for device to become idle!");

		CleanupSwapChain();

		vkDestroySampler(device, textureSamplers.linearRepeat, NULL);
		vkDestroySampler(device, textureSamplers.nearestRepeat, NULL);
		vkDestroySampler(device, textureSamplers.linearNoRepeat, NULL);
		vkDestroySampler(device, textureSamplers.nearestNoRepeat, NULL);
		for (size_t textureIndex = 0; textureIndex < textures.usedSlots; textureIndex++)
		{
			vkDestroyImageView(device, *(VkImageView *)ListGet(texturesImageView, textureIndex), NULL);
			vkDestroyImage(device, ((Texture *)ListGet(textures, textureIndex))->image, NULL);
		}
		vkFreeMemory(device, textureMemory.memory, NULL);

		CleanupColorImage();
		CleanupDepthImage();

		vkDestroyPipelineCache(device, pipelineCache, NULL);
		CleanupPipeline();

		vkDestroyRenderPass(device, renderPass, NULL);

		vkDestroyDescriptorPool(device, descriptorPool, NULL);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);

		if (!DestroyBuffer(&buffers.local))
		{
			return false;
		}
		if (!DestroyBuffer(&buffers.shared))
		{
			return false;
		}

		CleanupSyncObjects();

		vkDestroyCommandPool(device, graphicsCommandPool, NULL);
		vkDestroyCommandPool(device, transferCommandPool, NULL);
	}

	vkDestroyDevice(device, NULL);

	if (instance)
	{
		vkDestroySurfaceKHR(instance, surface, NULL);
	}

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

inline uint8_t VK_GetSampleCountFlags()
{
	return physicalDevice.properties.limits.framebufferColorSampleCounts &
		   physicalDevice.properties.limits.framebufferDepthSampleCounts &
		   0xF;
}

bool VK_LoadLevelWalls(const Level *level)
{
	void *data;
	uint32_t skyVertexCount = 0;

	if (level->ceilingTextureIndex == -1)
	{
		skyVertexCount = skyModel->packedVertsUvsCount;
		buffers.walls.skyIndexCount = skyModel->packedIndicesCount;
		buffers.walls.wallCount = level->walls.usedSlots + 1;
	} else
	{
		buffers.walls.skyIndexCount = 0;
		buffers.walls.wallCount = level->walls.usedSlots + 2;
	}

	if (buffers.walls.wallCount > buffers.walls.maxWallCount)
	{
		buffers.walls.maxWallCount = buffers.walls.wallCount;

		if (buffers.walls.verticesOffset <= buffers.walls.indicesOffset &&
			buffers.walls.indicesOffset == buffers.walls.verticesOffset + buffers.walls.vertexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.verticesOffset,
									buffers.walls.vertexSize + buffers.walls.indexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(UiVertex) * skyModel->packedVertsUvsCount +
											sizeof(uint32_t) * skyModel->packedIndicesCount,
									true,
									NULL))
			{
				return false;
			}
		} else if (buffers.walls.indicesOffset < buffers.walls.verticesOffset &&
				   buffers.walls.verticesOffset == buffers.walls.indicesOffset + buffers.walls.indexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.indicesOffset,
									buffers.walls.indexSize + buffers.walls.vertexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(UiVertex) * skyModel->packedVertsUvsCount +
											sizeof(uint32_t) * skyModel->packedIndicesCount,
									true,
									NULL))
			{
				return false;
			}
		} else
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.verticesOffset,
									buffers.walls.vertexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(UiVertex) * skyModel->packedVertsUvsCount,
									true,
									NULL))
			{
				return false;
			}
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.indicesOffset,
									buffers.walls.indexSize,
									sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(uint32_t) * skyModel->packedIndicesCount,
									true,
									NULL))
			{
				return false;
			}
		}
	}

	WallVertex vertices[buffers.walls.wallCount * 4 + skyVertexCount];
	uint32_t indices[buffers.walls.wallCount * 6 + buffers.walls.skyIndexCount];

	vertices[0 + skyVertexCount].x = -100;
	vertices[0 + skyVertexCount].y = -0.5f;
	vertices[0 + skyVertexCount].z = -100;
	vertices[0 + skyVertexCount].u = -100;
	vertices[0 + skyVertexCount].v = -100;
	vertices[0 + skyVertexCount].textureIndex = TextureIndex(wallTextures[level->floorTextureIndex]);

	vertices[1 + skyVertexCount].x = 100;
	vertices[1 + skyVertexCount].y = -0.5f;
	vertices[1 + skyVertexCount].z = -100;
	vertices[1 + skyVertexCount].u = 100;
	vertices[1 + skyVertexCount].v = -100;
	vertices[1 + skyVertexCount].textureIndex = TextureIndex(wallTextures[level->floorTextureIndex]);

	vertices[2 + skyVertexCount].x = 100;
	vertices[2 + skyVertexCount].y = -0.5f;
	vertices[2 + skyVertexCount].z = 100;
	vertices[2 + skyVertexCount].u = 100;
	vertices[2 + skyVertexCount].v = 100;
	vertices[2 + skyVertexCount].textureIndex = TextureIndex(wallTextures[level->floorTextureIndex]);

	vertices[3 + skyVertexCount].x = -100;
	vertices[3 + skyVertexCount].y = -0.5f;
	vertices[3 + skyVertexCount].z = 100;
	vertices[3 + skyVertexCount].u = -100;
	vertices[3 + skyVertexCount].v = 100;
	vertices[3 + skyVertexCount].textureIndex = TextureIndex(wallTextures[level->floorTextureIndex]);

	indices[0 + buffers.walls.skyIndexCount] = 0 + buffers.walls.skyIndexCount;
	indices[1 + buffers.walls.skyIndexCount] = 1 + buffers.walls.skyIndexCount;
	indices[2 + buffers.walls.skyIndexCount] = 2 + buffers.walls.skyIndexCount;
	indices[3 + buffers.walls.skyIndexCount] = 0 + buffers.walls.skyIndexCount;
	indices[4 + buffers.walls.skyIndexCount] = 2 + buffers.walls.skyIndexCount;
	indices[5 + buffers.walls.skyIndexCount] = 3 + buffers.walls.skyIndexCount;

	if (level->ceilingTextureIndex == -1)
	{
		for (uint32_t i = 0; i < skyModel->packedVertsUvsCount; i++)
		{
			memcpy(&vertices[i], &skyModel->packedVertsUvs[i * 8], sizeof(float) * 5);
			vertices[i].textureIndex = TextureIndex(TEXTURE("level_sky"));
		}
		memcpy(indices, skyModel->packedIndices, sizeof(uint32_t) * skyModel->packedIndicesCount);
	} else
	{
		vertices[4].x = -100;
		vertices[4].y = 0.5f;
		vertices[4].z = -100;
		vertices[4].u = -100;
		vertices[4].v = -100;
		vertices[4].textureIndex = TextureIndex(wallTextures[level->ceilingTextureIndex]);

		vertices[5].x = 100;
		vertices[5].y = 0.5f;
		vertices[5].z = -100;
		vertices[5].u = 100;
		vertices[5].v = -100;
		vertices[5].textureIndex = TextureIndex(wallTextures[level->ceilingTextureIndex]);

		vertices[6].x = 100;
		vertices[6].y = 0.5f;
		vertices[6].z = 100;
		vertices[6].u = 100;
		vertices[6].v = 100;
		vertices[6].textureIndex = TextureIndex(wallTextures[level->ceilingTextureIndex]);

		vertices[7].x = -100;
		vertices[7].y = 0.5f;
		vertices[7].z = 100;
		vertices[7].u = -100;
		vertices[7].v = 100;
		vertices[7].textureIndex = TextureIndex(wallTextures[level->ceilingTextureIndex]);

		indices[6] = 4;
		indices[7] = 5;
		indices[8] = 6;
		indices[9] = 4;
		indices[10] = 6;
		indices[11] = 7;
	}

	for (uint32_t i = !skyVertexCount ? 2 : 1; i < buffers.walls.wallCount; i++)
	{
		const Wall *wall = ListGet(level->walls, i - (!skyVertexCount ? 2 : 1));
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {(float)wall->a.x, (float)wall->a.y};
		const vec2 endVertex = {(float)wall->b.x, (float)wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const vec2 endUV = {(float)(wall->uvScale * wall->length + wall->uvOffset), 1};

		vertices[4 * i + skyVertexCount].x = startVertex[0];
		vertices[4 * i + skyVertexCount].y = halfHeight;
		vertices[4 * i + skyVertexCount].z = startVertex[1];
		vertices[4 * i + skyVertexCount].u = startUV[0];
		vertices[4 * i + skyVertexCount].v = startUV[1];
		vertices[4 * i + skyVertexCount].textureIndex = TextureIndex(wall->tex);

		vertices[4 * i + 1 + skyVertexCount].x = endVertex[0];
		vertices[4 * i + 1 + skyVertexCount].y = halfHeight;
		vertices[4 * i + 1 + skyVertexCount].z = endVertex[1];
		vertices[4 * i + 1 + skyVertexCount].u = endUV[0];
		vertices[4 * i + 1 + skyVertexCount].v = startUV[1];
		vertices[4 * i + 1 + skyVertexCount].textureIndex = TextureIndex(wall->tex);

		vertices[4 * i + 2 + skyVertexCount].x = endVertex[0];
		vertices[4 * i + 2 + skyVertexCount].y = -halfHeight;
		vertices[4 * i + 2 + skyVertexCount].z = endVertex[1];
		vertices[4 * i + 2 + skyVertexCount].u = endUV[0];
		vertices[4 * i + 2 + skyVertexCount].v = endUV[1];
		vertices[4 * i + 2 + skyVertexCount].textureIndex = TextureIndex(wall->tex);

		vertices[4 * i + 3 + skyVertexCount].x = startVertex[0];
		vertices[4 * i + 3 + skyVertexCount].y = -halfHeight;
		vertices[4 * i + 3 + skyVertexCount].z = startVertex[1];
		vertices[4 * i + 3 + skyVertexCount].u = startUV[0];
		vertices[4 * i + 3 + skyVertexCount].v = endUV[1];
		vertices[4 * i + 3 + skyVertexCount].textureIndex = TextureIndex(wall->tex);

		indices[6 * i + buffers.walls.skyIndexCount] = i * 4 + buffers.walls.skyIndexCount;
		indices[6 * i + 1 + buffers.walls.skyIndexCount] = i * 4 + 1 + buffers.walls.skyIndexCount;
		indices[6 * i + 2 + buffers.walls.skyIndexCount] = i * 4 + 2 + buffers.walls.skyIndexCount;
		indices[6 * i + 3 + buffers.walls.skyIndexCount] = i * 4 + buffers.walls.skyIndexCount;
		indices[6 * i + 4 + buffers.walls.skyIndexCount] = i * 4 + 2 + buffers.walls.skyIndexCount;
		indices[6 * i + 5 + buffers.walls.skyIndexCount] = i * 4 + 3 + buffers.walls.skyIndexCount;
	}

	MemoryInfo memoryInfo = {
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};
	const MemoryAllocationInfo allocationInfo = {
		.memoryInfo = &memoryInfo,
		.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	Buffer stagingBuffer = {
		.memoryAllocationInfo = allocationInfo,
		.size = buffers.walls.vertexSize + buffers.walls.indexSize,
	};
	if (!CreateBuffer(&stagingBuffer, true))
	{
		return false;
	}

	VulkanTest(vkMapMemory(device, memoryInfo.memory, 0, stagingBuffer.size, 0, &data),
			   "Failed to map wall staging buffer memory!");

	memcpy(data, vertices, sizeof(WallVertex) * (buffers.walls.wallCount * 4 + skyVertexCount));
	memcpy(data + buffers.walls.vertexSize,
		   indices,
		   sizeof(uint32_t) * (buffers.walls.wallCount * 6 + buffers.walls.skyIndexCount));
	vkUnmapMemory(device, memoryInfo.memory);

	const VkBufferCopy regions[] = {
		{
			.srcOffset = 0,
			.dstOffset = buffers.walls.verticesOffset,
			.size = buffers.walls.vertexSize,
		},
		{
			.srcOffset = buffers.walls.vertexSize,
			.dstOffset = buffers.walls.indicesOffset,
			.size = buffers.walls.indexSize,
		},
	};
	if (!CopyBuffer(stagingBuffer.buffer, buffers.walls.bufferInfo->buffer, 2, regions))
	{
		return false;
	}

	loadedLevel = level;

	return DestroyBuffer(&stagingBuffer);
}

bool VK_DrawColoredQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const uint32_t color)
{
	return DrawRectInternal(VK_X_TO_NDC(x),
							VK_Y_TO_NDC(y),
							VK_X_TO_NDC(x + w),
							VK_Y_TO_NDC(y + h),
							0,
							0,
							0,
							0,
							color,
							-1);
}

bool VK_DrawColoredQuadsBatched(const float *vertices, const int32_t quadCount, const uint32_t color)
{
	for (int32_t i = 0; i < quadCount; i++)
	{
		const uint32_t index = i * 8;
		const mat4 matrix = {
			{vertices[index + 0], vertices[index + 1], 0, 0},
			{vertices[index + 2], vertices[index + 3], 0, 0},
			{vertices[index + 4], vertices[index + 5], 0, 0},
			{vertices[index + 6], vertices[index + 7], 0, 0},
		};
		if (!DrawQuadInternal(matrix, color, -1))
		{
			return false;
		}
	}

	return true;
}

bool VK_DrawTexturedQuad(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const char *texture)
{
	return DrawRectInternal(VK_X_TO_NDC(x),
							VK_Y_TO_NDC(y),
							VK_X_TO_NDC(x + w),
							VK_Y_TO_NDC(y + h),
							0,
							0,
							1,
							1,
							0xFFFFFFFF,
							TextureIndex(texture));
}

bool VK_DrawTexturedQuadMod(const int32_t x,
							const int32_t y,
							const int32_t w,
							const int32_t h,
							const char *texture,
							const uint32_t color)
{
	return DrawRectInternal(VK_X_TO_NDC(x),
							VK_Y_TO_NDC(y),
							VK_X_TO_NDC(x + w),
							VK_Y_TO_NDC(y + h),
							0,
							0,
							1,
							1,
							color,
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
							   const char *texture)
{
	const Image *image = LoadImage(texture);

	const float startU = (float)regionX / (float)image->width;
	const float startV = (float)regionY / (float)image->height;

	return DrawRectInternal(VK_X_TO_NDC(x),
							VK_Y_TO_NDC(y),
							VK_X_TO_NDC(x + w),
							VK_Y_TO_NDC(y + h),
							startU,
							startV,
							startU + (float)regionW / (float)image->width,
							startV + (float)regionH / (float)image->height,
							0xFFFFFFFF,
							imageAssetIdToIndexMap[image->id]);
}

bool VK_DrawTexturedQuadRegionMod(const int32_t x,
								  const int32_t y,
								  const int32_t w,
								  const int32_t h,
								  const int32_t regionX,
								  const int32_t regionY,
								  const int32_t regionW,
								  const int32_t regionH,
								  const char *texture,
								  const uint32_t color)
{
	const Image *image = LoadImage(texture);

	const float startU = (float)regionX / (float)image->width;
	const float startV = (float)regionY / (float)image->height;

	return DrawRectInternal(VK_X_TO_NDC(x),
							VK_Y_TO_NDC(y),
							VK_X_TO_NDC(x + w),
							VK_Y_TO_NDC(y + h),
							startU,
							startV,
							startU + (float)regionW / (float)image->width,
							startV + (float)regionH / (float)image->height,
							color,
							imageAssetIdToIndexMap[image->id]);
}

bool VK_DrawTexturedQuadsBatched(const float *vertices,
								 const int32_t quadCount,
								 const char *texture,
								 const uint32_t color)
{
	for (int32_t i = 0; i < quadCount; i++)
	{
		const uint32_t index = i * 16;
		const mat4 matrix = {
			{
				vertices[index + 0],
				vertices[index + 1],
				vertices[index + 2],
				vertices[index + 3],
			},
			{
				vertices[index + 4],
				vertices[index + 5],
				vertices[index + 6],
				vertices[index + 7],
			},
			{
				vertices[index + 8],
				vertices[index + 9],
				vertices[index + 10],
				vertices[index + 11],
			},
			{
				vertices[index + 12],
				vertices[index + 13],
				vertices[index + 14],
				vertices[index + 15],
			},
		};

		if (!DrawQuadInternal(matrix, color, TextureIndex(texture)))
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
			{
				VK_X_TO_NDC(-dy / distance + (float)startX),
				VK_Y_TO_NDC(dx / distance + (float)startY),
				0,
				0,
			},
			{
				VK_X_TO_NDC(-dy / distance + (float)endX),
				VK_Y_TO_NDC(dx / distance + (float)endY),
				0,
				0,
			},
			{
				VK_X_TO_NDC(endX),
				VK_Y_TO_NDC(endY),
				0,
				0,
			},
			{
				VK_X_TO_NDC(startX),
				VK_Y_TO_NDC(startY),
				0,
				0,
			},
		};

		return DrawQuadInternal(matrix, color, -1);
	}

	const float size = thickness / 2;

	const mat4 matrix = {
		{
			VK_X_TO_NDC(-size * dy / distance + (float)startX),
			VK_Y_TO_NDC(size * dx / distance + (float)startY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(-size * dy / distance + (float)endX),
			VK_Y_TO_NDC(size * dx / distance + (float)endY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(size * dy / distance + (float)endX),
			VK_Y_TO_NDC(-size * dx / distance + (float)endY),
			0,
			0,
		},
		{
			VK_X_TO_NDC(size * dy / distance + (float)startX),
			VK_Y_TO_NDC(-size * dx / distance + (float)startY),
			0,
			0,
		},
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
	if (!VK_DrawLine(x, y, x + w, y, thickness, color))
	{
		return false;
	}
	if (!VK_DrawLine(x + w, y, x + w, y + h, thickness, color))
	{
		return false;
	}
	if (!VK_DrawLine(x + w, y + h, x, y + h, thickness, color))
	{
		return false;
	}
	if (!VK_DrawLine(x, y + h, x, y, thickness, color))
	{
		return false;
	}

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

void VK_SetTexParams(const char *texture, const bool linear, const bool repeat)
{
	const uint32_t textureIndex = TextureIndex(texture);
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo imageInfo = {
			.sampler = textureSamplers.nearestNoRepeat,
			.imageView = *(VkImageView *)ListGet(texturesImageView, textureIndex),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = descriptorSets[i],
			.dstBinding = 1,
			.dstArrayElement = textureIndex,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &imageInfo,
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL,
		};
		vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, NULL);
	}
}
