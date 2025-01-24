//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "../../../Structs/GlobalState.h"
#include "../../CommonAssets.h"
#include "../../Core/MathEx.h"
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

		VulkanTestReturnResult(ResizeUiBuffer(), "Failed to resize UI buffer!");

		buffers.ui.shouldResize = false;
	}

	VulkanTestReturnResult(CopyBuffers(loadedLevel), "Failed to copy buffers!");

	VulkanTestReturnResult(BeginRenderPass(commandBuffers[currentFrame], swapchainImageIndex),
						   "Failed to begin render pass!");

	const GlobalState *g = GetState();
	if ((g->currentState == MAIN_STATE || g->currentState == PAUSE_STATE) && buffers.walls.wallCount > 0)
	{
		vkCmdPushConstants(commandBuffers[currentFrame],
						   pipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
						   0,
						   sizeof(PushConstants),
						   &pushConstants);

		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.walls);

		vkCmdBindDescriptorSets(commandBuffers[currentFrame],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayout,
								0,
								1,
								&descriptorSets[currentFrame],
								0,
								NULL);

		vkCmdBindVertexBuffers(commandBuffers[currentFrame],
							   0,
							   2,
							   (VkBuffer[]){buffers.walls.bufferInfo->buffer, buffers.walls.bufferInfo->buffer},
							   (VkDeviceSize[]){buffers.walls.shadowOffset,
												buffers.walls.vertexOffset});

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.walls.bufferInfo->buffer,
							 buffers.walls.indexOffset,
							 VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffers[currentFrame],
						 6 + (loadedLevel->hasCeiling ? 6 : buffers.walls.skyIndexCount),
						 1,
						 0,
						 0,
						 0);

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.walls.bufferInfo->buffer,
							 buffers.walls.shadowOffset + sizeof(ShadowVertex) * buffers.walls.shadowCount * 4,
							 VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffers[currentFrame],
						 buffers.walls.shadowCount * 6,
						 1,
						 0,
						 0,
						 0x53484457); // 0x53484457 is "SHDW", to encode that we are drawing the shadows

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.walls.bufferInfo->buffer,
							 buffers.walls.indexOffset +
									 sizeof(uint32_t) *
											 (6 + (loadedLevel->hasCeiling ? 6 : buffers.walls.skyIndexCount)),
							 VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffers[currentFrame],
						 buffers.walls.wallCount * 6 - (loadedLevel->hasCeiling ? 12 : 6),
						 1,
						 0,
						 0,
						 0x57414C4C); // 0x57414C4C is "WALL", to encode that we are drawing the walls

		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.actors);

		vkCmdBindVertexBuffers(commandBuffers[currentFrame],
							   0,
							   2,
							   (VkBuffer[]){buffers.actors.bufferInfo->buffer, buffers.actors.bufferInfo->buffer},
							   (VkDeviceSize[]){buffers.actors.vertexOffset, buffers.actors.instanceDataOffset});

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.actors.bufferInfo->buffer,
							 buffers.actors.indexOffset,
							 VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[currentFrame],
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								pipelineLayout,
								0,
								1,
								&descriptorSets[currentFrame],
								0,
								NULL);

		vkCmdDrawIndexedIndirect(commandBuffers[currentFrame],
								 buffers.actors.bufferInfo->buffer,
								 buffers.actors.drawInfoOffset,
								 buffers.actors.models.loadedModelIds.length + buffers.actors.walls.count,
								 sizeof(VkDrawIndexedIndirectCommand));
	}

	vkCmdNextSubpass(commandBuffers[currentFrame], VK_SUBPASS_CONTENTS_INLINE);

	if (buffers.ui.quadCount > 0)
	{
		vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.ui);

		vkCmdBindVertexBuffers(commandBuffers[currentFrame],
							   0,
							   1,
							   &buffers.ui.bufferInfo->buffer,
							   &buffers.ui.vertexOffset);

		vkCmdBindIndexBuffer(commandBuffers[currentFrame],
							 buffers.ui.bufferInfo->buffer,
							 buffers.ui.indexOffset,
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
	pushConstants.position[0] = (float)loadedLevel->player.pos.x;
	pushConstants.position[1] = (float)loadedLevel->player.pos.y;
	pushConstants.yaw = camera->yaw + 1.5f * PIf;
	UpdateTranslationMatrix(camera);
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
		for (size_t textureIndex = 0; textureIndex < textures.length; textureIndex++)
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

	free(swapChainSupport.formats);
	free(swapChainSupport.presentMode);
	free(swapChainImages);
	free(swapChainImageViews);
	free(swapChainFramebuffers);

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
	const Model *_skyModel = !skyModel ? LoadModel(MODEL("model_sky")) : skyModel;
	if (level->hasCeiling)
	{
		buffers.walls.skyIndexCount = 0;
		buffers.walls.wallCount = level->walls.length + 2;
	} else
	{
		skyVertexCount = _skyModel->packedVertsUvsNormalCount;
		buffers.walls.skyIndexCount = _skyModel->packedIndicesCount;
		buffers.walls.wallCount = level->walls.length + 1;
	}

	if (buffers.walls.wallCount > buffers.walls.maxWallCount)
	{
		const VkDeviceSize wallVertexSize = sizeof(WallVertex) *
											(buffers.walls.maxWallCount * 4 + _skyModel->packedVertsUvsNormalCount);
		const VkDeviceSize wallIndexSize = sizeof(uint32_t) *
										   (buffers.walls.maxWallCount * 6 + _skyModel->packedIndicesCount);

		buffers.walls.maxWallCount = buffers.walls.wallCount;

		if (buffers.walls.vertexOffset <= buffers.walls.indexOffset &&
			buffers.walls.indexOffset == buffers.walls.vertexOffset + wallVertexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.vertexOffset,
									wallVertexSize + wallIndexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(UiVertex) * _skyModel->packedVertsUvsNormalCount +
											sizeof(uint32_t) * _skyModel->packedIndicesCount,
									true))
			{
				return false;
			}
		} else if (buffers.walls.indexOffset < buffers.walls.vertexOffset &&
				   buffers.walls.vertexOffset == buffers.walls.indexOffset + wallIndexSize)
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.indexOffset,
									wallIndexSize + wallVertexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(UiVertex) * _skyModel->packedVertsUvsNormalCount +
											sizeof(uint32_t) * _skyModel->packedIndicesCount,
									true))
			{
				return false;
			}
		} else
		{
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.vertexOffset,
									wallVertexSize,
									sizeof(UiVertex) * buffers.walls.maxWallCount * 4 +
											sizeof(UiVertex) * _skyModel->packedVertsUvsNormalCount,
									true))
			{
				return false;
			}
			if (!ResizeBufferRegion(&buffers.local,
									buffers.walls.indexOffset,
									wallIndexSize,
									sizeof(uint32_t) * buffers.walls.maxWallCount * 6 +
											sizeof(uint32_t) * _skyModel->packedIndicesCount,
									true))
			{
				return false;
			}
		}
	}

	ListClear(&buffers.actors.models.loadedModelIds);
	ListClear(&buffers.actors.models.modelCounts);
	memset(&buffers.actors.models, 0, sizeof(ModelActorBuffer));
	memset(&buffers.actors.walls, 0, sizeof(WallActorBuffer));
	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			if (!actor->actorWall)
			{
				continue;
			}
			buffers.actors.walls.count++;
		} else
		{
			size_t index = ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			if (index == -1)
			{
				index = buffers.actors.models.loadedModelIds.length;
				ListAdd(&buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
				buffers.actors.models.vertexCount += actor->actorModel->packedVertsUvsNormalCount;
				buffers.actors.models.indexCount += actor->actorModel->packedIndicesCount;
			}
			if (index < buffers.actors.models.modelCounts.length)
			{
				buffers.actors.models.modelCounts.data[index]++;
			} else
			{
				ListAdd(&buffers.actors.models.modelCounts, (void *)1);
			}
		}
		if (actor->showShadow)
		{
			buffers.walls.shadowCount++;
		}
	}

	if (sizeof(ActorVertex) * buffers.actors.models.vertexCount > buffers.actors.models.vertexSize ||
		sizeof(uint32_t) * buffers.actors.models.indexCount > buffers.actors.models.indexSize ||
		sizeof(ActorVertex) * buffers.actors.walls.count * 4 > buffers.actors.walls.vertexSize ||
		sizeof(uint32_t) * buffers.actors.walls.count * 6 > buffers.actors.walls.indexSize)
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
		if (!ResizeActorBuffer())
		{
			return false;
		}
	}

	WallVertex *wallVertices = calloc(buffers.walls.wallCount * 4 + skyVertexCount, sizeof(WallVertex));
	uint32_t *wallIndices = calloc(buffers.walls.wallCount * 6 + buffers.walls.skyIndexCount, sizeof(uint32_t));

	ActorVertex *actorVertices = calloc(buffers.actors.models.vertexCount, sizeof(ActorVertex));
	uint32_t *actorIndices = calloc(buffers.actors.models.indexCount, sizeof(uint32_t));

	LoadWalls(level, _skyModel, wallVertices, wallIndices, skyVertexCount);
	LoadActorModels(level, actorVertices, actorIndices);

	const VkDeviceSize wallVertexSize = sizeof(WallVertex) *
										(buffers.walls.maxWallCount * 4 + _skyModel->packedVertsUvsNormalCount);
	const VkDeviceSize wallIndexSize = sizeof(uint32_t) *
									   (buffers.walls.maxWallCount * 6 + _skyModel->packedIndicesCount);

	MemoryInfo memoryInfo = {
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};
	const MemoryAllocationInfo allocationInfo = {
		.memoryInfo = &memoryInfo,
		.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	};
	Buffer stagingBuffer = {
		.memoryAllocationInfo = allocationInfo,
		.size = wallVertexSize + wallIndexSize + buffers.actors.models.vertexSize + buffers.actors.models.indexSize,
	};
	if (!CreateBuffer(&stagingBuffer, true))
	{
		free(wallVertices);
		free(wallIndices);
		free(actorVertices);
		free(actorIndices);

		return false;
	}

	VulkanTest(vkMapMemory(device, memoryInfo.memory, 0, stagingBuffer.size, 0, &data),
			   "Failed to map wall staging buffer memory!");

	memcpy(data, wallVertices, sizeof(WallVertex) * (buffers.walls.wallCount * 4 + skyVertexCount));
	memcpy(data + wallVertexSize,
		   wallIndices,
		   sizeof(uint32_t) * (buffers.walls.wallCount * 6 + buffers.walls.skyIndexCount));
	memcpy(data + wallVertexSize + wallIndexSize,
		   actorVertices,
		   sizeof(ActorVertex) * buffers.actors.models.vertexCount);
	memcpy(data + wallVertexSize + wallIndexSize + buffers.actors.models.vertexSize,
		   actorIndices,
		   sizeof(uint32_t) * buffers.actors.models.indexCount);
	vkUnmapMemory(device, memoryInfo.memory);

	const VkBufferCopy regions[] = {
		{
			.srcOffset = 0,
			.dstOffset = buffers.walls.vertexOffset,
			.size = wallVertexSize,
		},
		{
			.srcOffset = wallVertexSize,
			.dstOffset = buffers.walls.indexOffset,
			.size = wallIndexSize,
		},
		{
			.srcOffset = wallVertexSize + wallIndexSize,
			.dstOffset = buffers.actors.vertexOffset,
			.size = buffers.actors.models.vertexSize,
		},
		{
			.srcOffset = wallVertexSize + wallIndexSize + buffers.actors.models.vertexSize,
			.dstOffset = buffers.actors.indexOffset,
			.size = buffers.actors.models.indexSize,
		},
	};
	if (!CopyBuffer(stagingBuffer.buffer, buffers.local.buffer, !buffers.actors.models.vertexSize ? 2 : 4, regions))
	{
		return false;
	}

	loadedLevel = level;
	pushConstants.skyVertexCount = loadedLevel->hasCeiling ? 0 : skyModel->packedVertsUvsNormalCount;
	pushConstants.skyTextureIndex = TextureIndex(loadedLevel->ceilOrSkyTex);
	pushConstants.shadowTextureIndex = TextureIndex(TEXTURE("vfx_shadow"));
	pushConstants.fogStart = loadedLevel->fogStart;
	pushConstants.fogEnd = loadedLevel->fogEnd;
	pushConstants.fogColor = loadedLevel->fogColor;

	free(wallVertices);
	free(wallIndices);
	free(actorVertices);
	free(actorIndices);

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
}

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
			.dstBinding = 0,
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
