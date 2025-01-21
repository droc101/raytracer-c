//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>

#include "../../CommonAssets.h"
#include "../../Core/Error.h"
#include "../RenderingHelpers.h"
#include "VulkanInternal.h"
#include "VulkanResources.h"

#pragma region variables
SDL_Window *vk_window;
bool minimized = false;

VkInstance instance = VK_NULL_HANDLE;
VkSurfaceKHR surface;
PhysicalDevice physicalDevice;
QueueFamilyIndices queueFamilyIndices;
SwapChainSupportDetails swapChainSupport;
VkDevice device = NULL;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkQueue transferQueue;
VkSwapchainKHR swapChain = VK_NULL_HANDLE;
VkImage *swapChainImages;
uint32_t swapChainCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews = NULL;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkPipelineCache pipelineCache = VK_NULL_HANDLE;
Pipelines pipelines = {.walls = VK_NULL_HANDLE, .actors = VK_NULL_HANDLE, .ui = VK_NULL_HANDLE};
VkFramebuffer *swapChainFramebuffers = NULL;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
bool framebufferResized = false;
uint8_t currentFrame = 0;
uint32_t swapchainImageIndex;
MemoryPools memoryPools = {
	{
		.size = 0,
		.mappedMemory = NULL,
		.memory = VK_NULL_HANDLE,
		.type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	},
	{
		.size = 0,
		.mappedMemory = NULL,
		.memory = VK_NULL_HANDLE,
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	},
};
Buffers buffers = {0};
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
List textures;
MemoryInfo textureMemory = {.type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
List texturesImageView;
uint32_t imageAssetIdToIndexMap[MAX_TEXTURES];
TextureSamplers textureSamplers = {
	.linearRepeat = VK_NULL_HANDLE,
	.nearestRepeat = VK_NULL_HANDLE,
	.linearNoRepeat = VK_NULL_HANDLE,
	.nearestNoRepeat = VK_NULL_HANDLE,
};
VkFormat depthImageFormat;
VkImage depthImage = VK_NULL_HANDLE;
VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
VkImageView depthImageView = VK_NULL_HANDLE;
VkImage colorImage = VK_NULL_HANDLE;
VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
VkImageView colorImageView = VK_NULL_HANDLE;
VkClearColorValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
uint16_t textureCount;
#pragma endregion variables

bool QuerySwapChainSupport(const VkPhysicalDevice pDevice)
{
	SwapChainSupportDetails details = {
		.formatCount = 0,
		.presentModeCount = 0,
		.formats = NULL,
		.presentMode = NULL,
		.capabilities = {0},
	};

	VulkanTest(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities),
			   "Failed to query Vulkan surface capabilities!");

	VulkanTest(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, NULL),
			   "Failed to query Vulkan surface color formats!");
	if (details.formatCount != 0)
	{
		details.formats = malloc(sizeof(*details.formats) * details.formatCount);
		VulkanTest(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, details.formats),
				   "Failed to query Vulkan surface color formats!");
	}

	VulkanTest(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, NULL),
			   "Failed to query Vulkan surface presentation modes!");
	if (details.presentModeCount != 0)
	{
		details.presentMode = malloc(sizeof(*details.presentMode) * details.presentModeCount);
		VulkanTest(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice,
															 surface,
															 &details.presentModeCount,
															 details.presentMode),
				   "Failed to query Vulkan surface presentation modes!");
	}
	swapChainSupport = details;

	return true;
}

bool CreateImageView(VkImageView *imageView,
					 const VkImage image,
					 const VkFormat format,
					 const VkImageAspectFlagBits aspectMask,
					 const uint8_t mipmapLevels,
					 const char *errorMessage)
{
	const VkComponentMapping componentMapping = {
		.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.a = VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	const VkImageSubresourceRange subresourceRange = {
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = mipmapLevels,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	const VkImageViewCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = componentMapping,
		.subresourceRange = subresourceRange,
	};

	VulkanTest(vkCreateImageView(device, &createInfo, NULL, imageView), errorMessage);

	return true;
}

VkShaderModule CreateShaderModule(const char *path)
{
	VkShaderModule shaderModule;
	const Asset *shader = DecompressAsset(path);

	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = shader->size - sizeof(uint32_t) * 4, // sizeof(uint32_t) * 4 is the asset header
		.pCode = (uint32_t *)shader->data,
	};

	VulkanTestWithReturn(vkCreateShaderModule(device, &createInfo, NULL, &shaderModule),
						 NULL,
						 "Failed to create shader module!");

	return shaderModule;
}

bool CreateImage(VkImage *image,
				 VkDeviceMemory *imageMemory,
				 const VkFormat format,
				 const VkExtent3D extent,
				 const uint8_t mipmapLevels,
				 const VkSampleCountFlags samples,
				 const VkImageUsageFlags usageFlags,
				 const char *imageType)
{
	uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											 ? queueFamilyIndices.transferFamily
											 : queueFamilyIndices.presentFamily;
			break;
		case 3:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			pQueueFamilyIndices[2] = queueFamilyIndices.transferFamily;
			break;
		default:
			VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!\n");
			return false;
	}

	const VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = extent,
		.mipLevels = mipmapLevels,
		.arrayLayers = 1,
		.samples = samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usageFlags,
		.sharingMode = queueFamilyIndices.families & QUEUE_FAMILY_PRESENTATION ? VK_SHARING_MODE_CONCURRENT
																			   : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VulkanTest(vkCreateImage(device, &imageInfo, NULL, image), "Failed to create Vulkan %s image!", imageType);

	if (!imageMemory)
	{
		return true; // If image memory is NULL, then allocation will be handled by the calling function
	}
	// Otherwise, allocate the memory for the image

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, *image, &memoryRequirements);
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ==
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			const VkDeviceSize size = memoryRequirements.alignment *
									  (VkDeviceSize)ceil((double)memoryRequirements.size /
														 (double)memoryRequirements.alignment);
			const VkMemoryAllocateInfo allocateInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = size,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocateInfo, NULL, imageMemory),
					   "Failed to allocate Vulkan %s image memory!",
					   imageType);
			break;
		}
	}

	VulkanTest(vkBindImageMemory(device, *image, *imageMemory, 0), "Failed to bind Vulkan %s image memory!", imageType);

	return true;
}

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, const VkCommandPool commandPool)
{
	const VkCommandBufferAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VulkanTest(vkAllocateCommandBuffers(device, &allocateInfo, (VkCommandBuffer *)commandBuffer),
			   "Failed to allocate Vulkan command buffers!");

	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};

	VulkanTest(vkBeginCommandBuffer(*commandBuffer, &beginInfo),
			   "Failed to start the recording of Vulkan command buffers!");

	return true;
}

bool EndCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandPool commandPool, const VkQueue queue)
{
	VulkanTest(vkEndCommandBuffer(commandBuffer), "Failed to finish the recording of Vulkan command buffers!");

	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.pWaitDstStageMask = 0,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = NULL,
	};

	VulkanTest(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE),
			   "Failed to submit Vulkan command buffers to queue!");

	VulkanTest(vkQueueWaitIdle(queue), "Failed to wait for Vulkan queue to become idle!");
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

	return true;
}

bool CreateBuffer(Buffer *buffer, const bool newAllocation)
{
	uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											 ? queueFamilyIndices.transferFamily
											 : queueFamilyIndices.presentFamily;
			break;
		case 3:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			pQueueFamilyIndices[2] = queueFamilyIndices.transferFamily;
			break;
		default:
			VulkanLogError("Failed to create VkSwapchainCreateInfoKHR due to invalid queueFamilyIndices!\n");
			return false;
	}

	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = buffer->size,
		.usage = buffer->memoryAllocationInfo.usageFlags,
		.sharingMode = queueFamilyIndices.families & QUEUE_FAMILY_PRESENTATION ? VK_SHARING_MODE_CONCURRENT
																			   : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
	};

	VulkanTest(vkCreateBuffer(device, &bufferInfo, NULL, &buffer->buffer), "Failed to create Vulkan buffer!");

	vkGetBufferMemoryRequirements(device, buffer->buffer, &buffer->memoryAllocationInfo.memoryRequirements);
	const VkDeviceSize memorySize = buffer->memoryAllocationInfo.memoryRequirements.alignment *
									(VkDeviceSize)
											ceil((double)buffer->memoryAllocationInfo.memoryRequirements.size /
												 (double)buffer->memoryAllocationInfo.memoryRequirements.alignment);

	buffer->memoryAllocationInfo.memoryInfo->size = memorySize;

	if (!newAllocation)
	{
		return true; // Allocation and binding will be handled elsewhere
	}

	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (buffer->memoryAllocationInfo.memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags &
			 buffer->memoryAllocationInfo.memoryInfo->type) == buffer->memoryAllocationInfo.memoryInfo->type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memorySize,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &buffer->memoryAllocationInfo.memoryInfo->memory),
					   "Failed to allocate Vulkan buffer memory!");

			VulkanTest(vkBindBufferMemory(device, buffer->buffer, buffer->memoryAllocationInfo.memoryInfo->memory, 0),
					   "Failed to bind Vulkan buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to find suitable memory type for buffer!\n");

	return false;
}

bool CopyBuffer(const VkBuffer srcBuffer,
				const VkBuffer dstBuffer,
				const uint32_t regionCount,
				const VkBufferCopy *regions)
{
	const VkCommandBuffer commandBuffer;
	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, regions);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	return true;
}

inline uint32_t TextureIndex(const char *texture)
{
	const uint32_t index = imageAssetIdToIndexMap[LoadImage(texture)->id];
	if (index == -1)
	{
		if (!LoadTexture(texture))
		{
			Error("Failed to load texture!");
		}
		return imageAssetIdToIndexMap[LoadImage(texture)->id];
	}
	return index;
}

void CleanupSwapChain()
{
	if (swapChainFramebuffers)
	{
		for (uint32_t i = 0; i < swapChainCount; i++)
		{
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
		}
	}
	if (swapChainImageViews)
	{
		for (uint32_t i = 0; i < swapChainCount; i++)
		{
			vkDestroyImageView(device, swapChainImageViews[i], NULL);
		}
	}
	vkDestroySwapchainKHR(device, swapChain, NULL);
}

void CleanupColorImage()
{
	vkDestroyImageView(device, colorImageView, NULL);
	vkDestroyImage(device, colorImage, NULL);
	vkFreeMemory(device, colorImageMemory, NULL);
}

void CleanupDepthImage()
{
	vkDestroyImageView(device, depthImageView, NULL);
	vkDestroyImage(device, depthImage, NULL);
	vkFreeMemory(device, depthImageMemory, NULL);
}

void CleanupPipeline()
{
	vkDestroyPipeline(device, pipelines.walls, NULL);
	vkDestroyPipeline(device, pipelines.actors, NULL);
	vkDestroyPipeline(device, pipelines.ui, NULL);
	vkDestroyPipelineLayout(device, pipelineLayout, NULL);
}

void CleanupSyncObjects()
{
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);

		vkDestroyFence(device, inFlightFences[i], NULL);
	}
}

bool RecreateSwapChain()
{
	VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for device to become idle!");

	CleanupSwapChain();
	CleanupColorImage();
	CleanupDepthImage();
	CleanupPipeline();
	CleanupSyncObjects();

	return CreateSwapChain() &&
		   CreateImageViews() &&
		   CreateGraphicsPipelines() &&
		   CreateColorImage() &&
		   CreateDepthImage() &&
		   CreateFramebuffers() &&
		   CreateSyncObjects();
}

bool DestroyBuffer(Buffer *buffer)
{
	VulkanTest(vkDeviceWaitIdle(device), "Failed to wait for device to become idle!");

	vkDestroyBuffer(device, buffer->buffer, NULL);
	buffer->buffer = VK_NULL_HANDLE;

	vkFreeMemory(device, buffer->memoryAllocationInfo.memoryInfo->memory, NULL);
	buffer->memoryAllocationInfo = (MemoryAllocationInfo){0};

	return true;
}

void LoadWalls(const Level *level,
			   const Model *skyModel,
			   WallVertex *vertices,
			   uint32_t *indices,
			   const uint32_t skyVertexCount)
{
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
		for (uint32_t i = 0; i < skyModel->packedVertsUvsNormalCount; i++)
		{
			memcpy(&vertices[i], &skyModel->packedVertsUvsNormal[i * 8], sizeof(float) * 5);
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
}

void LoadActorModels(const Level *level, ActorVertex *vertices, uint32_t *indices)
{
	size_t vertexOffset = 0;
	size_t indexOffset = 0;
	ListClear(&buffers.actors.models.loadedModelIds);
	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorModel)
		{
			continue;
		}
		if (ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id) == -1)
		{
			ListAdd(&buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			const size_t vertexSize = sizeof(*vertices) * actor->actorModel->packedVertsUvsNormalCount;
			const size_t indexSize = sizeof(*indices) * actor->actorModel->packedIndicesCount;
			memcpy(&vertices[vertexOffset], actor->actorModel->packedVertsUvsNormal, vertexSize);
			memcpy(&indices[vertexOffset], actor->actorModel->packedIndices, indexSize);
			vertexOffset += vertexSize;
			indexOffset += indexSize;
		}
	}
}

void LoadActorWalls(const Level *level, ActorVertex *vertices, uint32_t *indices)
{
	uint32_t wallCount = 0;
	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall || actor->actorModel != NULL)
		{
			continue;
		}
		const Wall *wall = actor->actorWall;
		const float halfHeight = wall->height / 2.0f;
		const vec2 startVertex = {(float)wall->a.x, (float)wall->a.y};
		const vec2 endVertex = {(float)wall->b.x, (float)wall->b.y};
		const vec2 startUV = {wall->uvOffset, 0};
		const float dx = (float)wall->b.x - (float)wall->a.x;
		const float dy = (float)wall->b.y - (float)wall->a.y;
		const float length = sqrtf(dx * dx + dy * dy);
		const vec2 endUV = {wall->uvScale * length + wall->uvOffset, 1};

		vertices[4 * wallCount].x = startVertex[0];
		vertices[4 * wallCount].y = halfHeight;
		vertices[4 * wallCount].z = startVertex[1];
		vertices[4 * wallCount].u = startUV[0];
		vertices[4 * wallCount].v = startUV[1];

		vertices[4 * wallCount + 1].x = endVertex[0];
		vertices[4 * wallCount + 1].y = halfHeight;
		vertices[4 * wallCount + 1].z = endVertex[1];
		vertices[4 * wallCount + 1].u = endUV[0];
		vertices[4 * wallCount + 1].v = startUV[1];

		vertices[4 * wallCount + 2].x = endVertex[0];
		vertices[4 * wallCount + 2].y = -halfHeight;
		vertices[4 * wallCount + 2].z = endVertex[1];
		vertices[4 * wallCount + 2].u = endUV[0];
		vertices[4 * wallCount + 2].v = endUV[1];

		vertices[4 * wallCount + 3].x = startVertex[0];
		vertices[4 * wallCount + 3].y = -halfHeight;
		vertices[4 * wallCount + 3].z = startVertex[1];
		vertices[4 * wallCount + 3].u = startUV[0];
		vertices[4 * wallCount + 3].v = endUV[1];

		indices[6 * wallCount] = wallCount * 4;
		indices[6 * wallCount + 1] = wallCount * 4 + 1;
		indices[6 * wallCount + 2] = wallCount * 4 + 2;
		indices[6 * wallCount + 3] = wallCount * 4;
		indices[6 * wallCount + 4] = wallCount * 4 + 2;
		indices[6 * wallCount + 5] = wallCount * 4 + 3;

		wallCount++;
	}
}

void LoadActorInstanceData(const Level *level, ActorInstanceData *instanceData)
{
	mat4 transformMatrix;
	uint32_t wallCount = 0;
	uint16_t *modelCounts = calloc(buffers.actors.models.loadedModelIds.length, sizeof(uint16_t));
	uint32_t *offsets = calloc(buffers.actors.models.loadedModelIds.length + 1, sizeof(uint32_t));
	for (size_t i = 1; i <= buffers.actors.models.loadedModelIds.length; i++)
	{
		offsets[i] = offsets[i - 1] + buffers.actors.models.modelCounts[i - 1] * sizeof(ActorInstanceData);
	}
	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall && !actor->actorModel)
		{
			continue;
		}
		ActorTransformMatrix(actor, &transformMatrix);
		if (actor->actorModel)
		{
			const size_t index = ListFind(buffers.actors.models.loadedModelIds, (void *)actor->actorModel->id);
			ActorInstanceData *offsetInstanceData = (void *)instanceData + offsets[index];

			memcpy(offsetInstanceData[modelCounts[index]].transform, transformMatrix, sizeof(mat4));
			offsetInstanceData[modelCounts[index]].textureIndex = TextureIndex(actor->actorModelTexture);

			modelCounts[index]++;
		} else if (actor->actorWall)
		{
			ActorInstanceData *offsetInstanceData = (void *)instanceData +
													offsets[buffers.actors.models.loadedModelIds.length];

			memcpy(offsetInstanceData[wallCount].transform, transformMatrix, sizeof(mat4));
			offsetInstanceData[wallCount].textureIndex = TextureIndex(actor->actorWall->tex);

			wallCount++;
		}
	}
	free(modelCounts);
	free(offsets);
}

void LoadActorDrawInfo(const Level *level, VkDrawIndexedIndirectCommand *drawInfo)
{
	uint32_t modelCount = 0;
	uint32_t wallCount = 0;
	for (size_t i = 0; i < buffers.actors.models.loadedModelIds.length; i++)
	{
		drawInfo[i].indexCount = GetModelFromId((size_t)ListGet(buffers.actors.models.loadedModelIds, i))
										 ->packedIndicesCount;
		drawInfo[i].instanceCount = buffers.actors.models.modelCounts[i];
		modelCount += buffers.actors.models.modelCounts[i];
	}
	for (size_t i = 0; i < level->actors.length; i++)
	{
		const Actor *actor = ListGet(level->actors, i);
		if (!actor->actorWall || actor->actorModel)
		{
			continue;
		}
		if (actor->actorWall)
		{
			const size_t index = wallCount + buffers.actors.models.loadedModelIds.length;
			drawInfo[index].indexCount = 6;
			drawInfo[index].instanceCount = 1;
			drawInfo[index].firstInstance = wallCount + modelCount;
			drawInfo[index].firstIndex = buffers.actors.models.indexCount + (int32_t)(wallCount * 6);
			drawInfo[index].vertexOffset = buffers.actors.models.vertexCount;
			wallCount++;
		}
	}
}

VkResult CopyBuffers(const Level *level)
{
	memcpy(buffers.ui.vertexStaging, buffers.ui.vertices, sizeof(UiVertex) * buffers.ui.quadCount * 4);
	memcpy(buffers.ui.indexStaging, buffers.ui.indices, sizeof(uint32_t) * buffers.ui.quadCount * 6);

	if (buffers.actors.models.loadedModelIds.length ||
		(buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize))
	{
		ActorVertex *actorVertices = calloc(1, buffers.actors.walls.vertexSize);
		uint32_t *actorIndices = calloc(1, buffers.actors.walls.indexSize);
		LoadActorWalls(level, actorVertices, actorIndices);
		memcpy(buffers.actors.walls.vertexStaging, actorVertices, buffers.actors.walls.vertexSize);
		memcpy(buffers.actors.walls.indexStaging, actorIndices, buffers.actors.walls.indexSize);
		free(actorVertices);
		free(actorIndices);

		ActorInstanceData *actorInstanceData = calloc(1, buffers.actors.instanceDataSize);
		LoadActorInstanceData(level, actorInstanceData);
		memcpy(buffers.actors.instanceDataStaging, actorInstanceData, buffers.actors.instanceDataSize);
		free(actorInstanceData);

		VkDrawIndexedIndirectCommand *actorDrawInfo = calloc(1, buffers.actors.drawInfoSize);
		LoadActorDrawInfo(level, actorDrawInfo);
		memcpy(buffers.actors.drawInfoStaging, actorDrawInfo, buffers.actors.drawInfoSize);
		free(actorDrawInfo);
	}

	// TODO: The transfer command buffer should be left open and multiple commands should be submitted, that way it can
	//  be handled according to if it should be or not. That will allow this function to be broken into multiple more
	//  manageable functions, as well as increase overall performance.
	if (buffers.ui.quadCount > 0 || (buffers.actors.models.loadedModelIds.length ||
									 (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)))
	{
		const VkCommandBuffer commandBuffer;
		if (!BeginCommandBuffer(&commandBuffer, transferCommandPool))
		{
			return VK_ERROR_UNKNOWN;
		}
		if (buffers.ui.quadCount > 0 && (buffers.actors.models.loadedModelIds.length ||
										 (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)))
		{
			if (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)
			{
				if (buffers.ui.stagingBufferInfo == &buffers.shared &&
					buffers.actors.stagingBufferInfo == &buffers.shared &&
					buffers.ui.bufferInfo == &buffers.local &&
					buffers.actors.bufferInfo == &buffers.local)
				{
					vkCmdCopyBuffer(commandBuffer,
									buffers.shared.buffer,
									buffers.local.buffer,
									6,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.ui.vertexStagingOffset,
											.dstOffset = buffers.ui.vertexOffset,
											.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
										},
										{
											.srcOffset = buffers.ui.indexStagingOffset,
											.dstOffset = buffers.ui.indexOffset,
											.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
										},
										{
											.srcOffset = buffers.actors.walls.vertexStagingOffset,
											.dstOffset = buffers.actors.walls.vertexOffset,
											.size = buffers.actors.walls.vertexSize,
										},
										{
											.srcOffset = buffers.actors.walls.indexStagingOffset,
											.dstOffset = buffers.actors.walls.indexOffset,
											.size = buffers.actors.walls.indexSize,
										},
										{
											.srcOffset = buffers.actors.instanceDataStagingOffset,
											.dstOffset = buffers.actors.instanceDataOffset,
											.size = buffers.actors.instanceDataSize,
										},
										{
											.srcOffset = buffers.actors.drawInfoStagingOffset,
											.dstOffset = buffers.actors.drawInfoOffset,
											.size = buffers.actors.drawInfoSize,
										},
									});
				} else
				{
					vkCmdCopyBuffer(commandBuffer,
									buffers.ui.stagingBufferInfo->buffer,
									buffers.ui.bufferInfo->buffer,
									2,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.ui.vertexStagingOffset,
											.dstOffset = buffers.ui.vertexOffset,
											.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
										},
										{
											.srcOffset = buffers.ui.indexStagingOffset,
											.dstOffset = buffers.ui.indexOffset,
											.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
										},
									});
					vkCmdCopyBuffer(commandBuffer,
									buffers.actors.stagingBufferInfo->buffer,
									buffers.actors.bufferInfo->buffer,
									4,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.actors.walls.vertexStagingOffset,
											.dstOffset = buffers.actors.walls.vertexOffset,
											.size = buffers.actors.walls.vertexSize,
										},
										{
											.srcOffset = buffers.actors.walls.indexStagingOffset,
											.dstOffset = buffers.actors.walls.indexOffset,
											.size = buffers.actors.walls.indexSize,
										},
										{
											.srcOffset = buffers.actors.instanceDataStagingOffset,
											.dstOffset = buffers.actors.instanceDataOffset,
											.size = buffers.actors.instanceDataSize,
										},
										{
											.srcOffset = buffers.actors.drawInfoStagingOffset,
											.dstOffset = buffers.actors.drawInfoOffset,
											.size = buffers.actors.drawInfoSize,
										},
									});
				}
			} else
			{
				if (buffers.ui.stagingBufferInfo == &buffers.shared &&
					buffers.actors.stagingBufferInfo == &buffers.shared &&
					buffers.ui.bufferInfo == &buffers.local &&
					buffers.actors.bufferInfo == &buffers.local)
				{
					vkCmdCopyBuffer(commandBuffer,
									buffers.shared.buffer,
									buffers.local.buffer,
									4,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.ui.vertexStagingOffset,
											.dstOffset = buffers.ui.vertexOffset,
											.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
										},
										{
											.srcOffset = buffers.ui.indexStagingOffset,
											.dstOffset = buffers.ui.indexOffset,
											.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
										},
										{
											.srcOffset = buffers.actors.instanceDataStagingOffset,
											.dstOffset = buffers.actors.instanceDataOffset,
											.size = buffers.actors.instanceDataSize,
										},
										{
											.srcOffset = buffers.actors.drawInfoStagingOffset,
											.dstOffset = buffers.actors.drawInfoOffset,
											.size = buffers.actors.drawInfoSize,
										},
									});
				} else
				{
					vkCmdCopyBuffer(commandBuffer,
									buffers.ui.stagingBufferInfo->buffer,
									buffers.ui.bufferInfo->buffer,
									2,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.ui.vertexStagingOffset,
											.dstOffset = buffers.ui.vertexOffset,
											.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
										},
										{
											.srcOffset = buffers.ui.indexStagingOffset,
											.dstOffset = buffers.ui.indexOffset,
											.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
										},
									});
					vkCmdCopyBuffer(commandBuffer,
									buffers.actors.stagingBufferInfo->buffer,
									buffers.actors.bufferInfo->buffer,
									2,
									(VkBufferCopy[]){
										{
											.srcOffset = buffers.actors.instanceDataStagingOffset,
											.dstOffset = buffers.actors.instanceDataOffset,
											.size = buffers.actors.instanceDataSize,
										},
										{
											.srcOffset = buffers.actors.drawInfoStagingOffset,
											.dstOffset = buffers.actors.drawInfoOffset,
											.size = buffers.actors.drawInfoSize,
										},
									});
				}
			}
		} else if (buffers.ui.quadCount > 0)
		{
			vkCmdCopyBuffer(commandBuffer,
							buffers.ui.stagingBufferInfo->buffer,
							buffers.ui.bufferInfo->buffer,
							2,
							(VkBufferCopy[]){
								{
									.srcOffset = buffers.ui.vertexStagingOffset,
									.dstOffset = buffers.ui.vertexOffset,
									.size = sizeof(UiVertex) * buffers.ui.quadCount * 4,
								},
								{
									.srcOffset = buffers.ui.indexStagingOffset,
									.dstOffset = buffers.ui.indexOffset,
									.size = sizeof(uint32_t) * buffers.ui.quadCount * 6,
								},
							});
		} else
		{
			if (buffers.actors.walls.vertexSize && buffers.actors.walls.indexSize)
			{
				vkCmdCopyBuffer(commandBuffer,
								buffers.actors.stagingBufferInfo->buffer,
								buffers.actors.bufferInfo->buffer,
								4,
								(VkBufferCopy[]){
									{
										.srcOffset = buffers.actors.walls.vertexStagingOffset,
										.dstOffset = buffers.actors.walls.vertexOffset,
										.size = buffers.actors.walls.vertexSize,
									},
									{
										.srcOffset = buffers.actors.walls.indexStagingOffset,
										.dstOffset = buffers.actors.walls.indexOffset,
										.size = buffers.actors.walls.indexSize,
									},
									{
										.srcOffset = buffers.actors.instanceDataStagingOffset,
										.dstOffset = buffers.actors.instanceDataOffset,
										.size = buffers.actors.instanceDataSize,
									},
									{
										.srcOffset = buffers.actors.drawInfoStagingOffset,
										.dstOffset = buffers.actors.drawInfoOffset,
										.size = buffers.actors.drawInfoSize,
									},
								});
			} else
			{
				vkCmdCopyBuffer(commandBuffer,
								buffers.actors.stagingBufferInfo->buffer,
								buffers.actors.bufferInfo->buffer,
								2,
								(VkBufferCopy[]){
									{
										.srcOffset = buffers.actors.instanceDataStagingOffset,
										.dstOffset = buffers.actors.instanceDataOffset,
										.size = buffers.actors.instanceDataSize,
									},
									{
										.srcOffset = buffers.actors.drawInfoStagingOffset,
										.dstOffset = buffers.actors.drawInfoOffset,
										.size = buffers.actors.drawInfoSize,
									},
								});
			}
		}


		if (!EndCommandBuffer(commandBuffer, transferCommandPool, transferQueue))
		{
			return VK_ERROR_UNKNOWN;
		}
	}

	return VK_SUCCESS;
}

void UpdateUniformBuffer(const Camera *camera, const uint32_t currentFrame)
{
	mat4 perspective;
	glm_perspective_lh_zo(glm_rad(camera->fov),
						  (float)swapChainExtent.width / (float)swapChainExtent.height,
						  NEAR_Z,
						  FAR_Z,
						  perspective);

	vec3 viewTarget = {cosf(camera->yaw), 0, sinf(camera->yaw)};

	// TODO roll and pitch might be messed up (test and fix as needed)
	glm_vec3_rotate(viewTarget, camera->roll, GLM_ZUP); // Roll
	glm_vec3_rotate(viewTarget, camera->pitch, GLM_XUP); // Pitch

	vec3 cameraPosition = {camera->x, camera->y, camera->z};
	glm_vec3_add(viewTarget, cameraPosition, viewTarget);

	mat4 view;
	glm_lookat_lh_zo(cameraPosition, viewTarget, (vec3){0.0f, -1.0f, 0.0f}, view);

	glm_mat4_mul(perspective, view, *buffers.translation[currentFrame].data);
}

VkResult BeginRenderPass(const VkCommandBuffer commandBuffer, const uint32_t imageIndex)
{
	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL,
	};

	VulkanTestReturnResult(vkBeginCommandBuffer(commandBuffer, &beginInfo),
						   "Failed to begin recording Vulkan command buffer!");

	const VkRect2D renderArea = {
		.offset = {0, 0},
		.extent = swapChainExtent,
	};
	const VkClearValue clearValues[] = {
		{.color = clearColor},
		{.depthStencil = {1, 0}},
	};
	const VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = renderPass,
		.framebuffer = swapChainFramebuffers[imageIndex],
		.renderArea = renderArea,
		.clearValueCount = 2,
		.pClearValues = clearValues,
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	return VK_SUCCESS;
}

VkResult EndRenderPass(const VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);

	VulkanTestReturnResult(vkEndCommandBuffer(commandBuffer), "Failed to record the Vulkan command buffer!");

	return VK_SUCCESS;
}

bool DrawRectInternal(const float ndcStartX,
					  const float ndcStartY,
					  const float ndcEndX,
					  const float ndcEndY,
					  const float startU,
					  const float startV,
					  const float endU,
					  const float endV,
					  const uint32_t color,
					  const uint32_t textureIndex)
{
	const mat4 matrix = {
		{ndcStartX, ndcStartY, startU, startV},
		{ndcEndX, ndcStartY, endU, startV},
		{ndcEndX, ndcEndY, endU, endV},
		{ndcStartX, ndcEndY, startU, endV},
	};
	return DrawQuadInternal(matrix, color, textureIndex);
}

bool DrawQuadInternal(const mat4 vertices_posXY_uvZW, const uint32_t color, const uint32_t textureIndex)
{
	GET_COLOR(color);

	if (buffers.ui.quadCount >= buffers.ui.maxQuads)
	{
		buffers.ui.maxQuads += 16;
		buffers.ui.shouldResize = true;

		UiVertex *newVertices = realloc(buffers.ui.vertices, sizeof(UiVertex) * buffers.ui.maxQuads * 4);
		if (!newVertices)
		{
			free(buffers.ui.vertices);
			buffers.ui.vertices = NULL;

			VulkanLogError("realloc of UI vertex buffer data pointer failed!\n");

			return false;
		}
		buffers.ui.vertices = newVertices;

		uint32_t *newIndices = realloc(buffers.ui.indices, sizeof(uint32_t) * buffers.ui.maxQuads * 6);
		if (!newIndices)
		{
			free(newVertices);
			free(buffers.ui.vertices);
			buffers.ui.vertices = NULL;

			free(buffers.ui.indices);
			buffers.ui.indices = NULL;

			VulkanLogError("realloc of UI index buffer data pointer failed!\n");

			return false;
		}
		buffers.ui.indices = newIndices;
	}

	buffers.ui.vertices[4 * buffers.ui.quadCount] = (UiVertex){
		.x = vertices_posXY_uvZW[0][0],
		.y = vertices_posXY_uvZW[0][1],
		.u = vertices_posXY_uvZW[0][2],
		.v = vertices_posXY_uvZW[0][3],
		.r = r,
		.g = g,
		.b = b,
		.a = a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 1] = (UiVertex){
		.x = vertices_posXY_uvZW[1][0],
		.y = vertices_posXY_uvZW[1][1],
		.u = vertices_posXY_uvZW[1][2],
		.v = vertices_posXY_uvZW[1][3],
		.r = r,
		.g = g,
		.b = b,
		.a = a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 2] = (UiVertex){
		.x = vertices_posXY_uvZW[2][0],
		.y = vertices_posXY_uvZW[2][1],
		.u = vertices_posXY_uvZW[2][2],
		.v = vertices_posXY_uvZW[2][3],
		.r = r,
		.g = g,
		.b = b,
		.a = a,
		.textureIndex = textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 3] = (UiVertex){
		.x = vertices_posXY_uvZW[3][0],
		.y = vertices_posXY_uvZW[3][1],
		.u = vertices_posXY_uvZW[3][2],
		.v = vertices_posXY_uvZW[3][3],
		.r = r,
		.g = g,
		.b = b,
		.a = a,
		.textureIndex = textureIndex,
	};

	buffers.ui.indices[6 * buffers.ui.quadCount] = buffers.ui.quadCount * 4;
	buffers.ui.indices[6 * buffers.ui.quadCount + 1] = buffers.ui.quadCount * 4 + 1;
	buffers.ui.indices[6 * buffers.ui.quadCount + 2] = buffers.ui.quadCount * 4 + 2;
	buffers.ui.indices[6 * buffers.ui.quadCount + 3] = buffers.ui.quadCount * 4;
	buffers.ui.indices[6 * buffers.ui.quadCount + 4] = buffers.ui.quadCount * 4 + 2;
	buffers.ui.indices[6 * buffers.ui.quadCount + 5] = buffers.ui.quadCount * 4 + 3;

	buffers.ui.quadCount++;

	return true;
}
