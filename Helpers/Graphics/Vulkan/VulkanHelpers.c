//
// Created by Noah on 11/23/2024.
//

#include "VulkanHelpers.h"
#include <cglm/clipspace/persp_lh_zo.h>
#include <cglm/clipspace/view_lh_zo.h>

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
Pipelines pipelines = {.walls = VK_NULL_HANDLE, .ui = VK_NULL_HANDLE};
VkFramebuffer *swapChainFramebuffers = NULL;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
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
		.memoryTypeBits = 0,
		.type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	},
	{
		.size = 0,
		.mappedMemory = NULL,
		.memory = VK_NULL_HANDLE,
		.memoryTypeBits = 0,
		.type = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	},
};
Buffers buffers = {0};
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
Image textures[TEXTURE_ASSET_COUNT];
VkDeviceMemory textureMemory = VK_NULL_HANDLE;
VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
uint32_t texturesAssetIDMap[ASSET_COUNT];
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

VkShaderModule CreateShaderModule(const uint32_t *code, const size_t size)
{
	VkShaderModule shaderModule;

	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = size - 16,
		.pCode = code,
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
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			break;
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		default:
			VulkanLogError("Failed to create VkImageCreateInfoKHR due to invalid queueFamilyIndices!");
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
		.sharingMode = queueFamilyIndices.familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
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

bool CreateBuffer(VkBuffer *buffer,
				  const VkDeviceSize size,
				  const VkBufferUsageFlags usageFlags,
				  const bool newAllocation,
				  MemoryAllocationInfo *allocationInfo)
{
	uint32_t pQueueFamilyIndices[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 2:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			pQueueFamilyIndices[1] = queueFamilyIndices.presentFamily;
			break;
		case 1:
			pQueueFamilyIndices[0] = queueFamilyIndices.graphicsFamily;
			break;
		default:
			VulkanLogError("Failed to create VkBufferCreateInfo due to invalid queueFamilyIndices!");
			return false;
	}

	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = usageFlags,
		.sharingMode = queueFamilyIndices.familyCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
	};

	VulkanTest(vkCreateBuffer(device, &bufferInfo, NULL, buffer), "Failed to create Vulkan buffer!");

	vkGetBufferMemoryRequirements(device, *buffer, &allocationInfo->memoryRequirements);
	const VkDeviceSize memorySize = allocationInfo->memoryRequirements.alignment *
									(VkDeviceSize)ceil((double)allocationInfo->memoryRequirements.size /
													   (double)allocationInfo->memoryRequirements.alignment);

	allocationInfo->offset = allocationInfo->memoryInfo->size;
	allocationInfo->memoryInfo->size += memorySize;
	allocationInfo->memoryInfo->memoryTypeBits |= allocationInfo->memoryRequirements.memoryTypeBits;

	if (!newAllocation)
	{
		return true; // Allocation and binding will be handled elsewhere
	}

	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (allocationInfo->memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & allocationInfo->memoryInfo->type) ==
					allocationInfo->memoryInfo->type)
		{
			const VkMemoryAllocateInfo allocInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = NULL,
				.allocationSize = memorySize,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocInfo, NULL, &allocationInfo->memoryInfo->memory),
					   "Failed to allocate Vulkan buffer memory!");

			VulkanTest(vkBindBufferMemory(device, *buffer, allocationInfo->memoryInfo->memory, 0),
					   "Failed to bind Vulkan buffer memory!");

			return true;
		}
	}

	VulkanLogError("Failed to find suitable memory type for buffer!");

	return false;
}

bool CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
	const VkCommandBuffer commandBuffer;
	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	const VkBufferCopy copyRegion = {0, 0, size};
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	return true;
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
		if (buffers.ui.quadCount >= buffers.ui.fallbackMaxQuads)
		{
			if (buffers.ui.fallbackMaxQuads)
			{
				buffers.ui.fallbackMaxQuads += 16;

				UiVertex *newVertices = realloc(buffers.ui.fallbackVertices,
												sizeof(UiVertex) * buffers.ui.fallbackMaxQuads * 4);
				if (!newVertices)
				{
					free(newVertices);
					free(buffers.ui.fallbackVertices);

					VulkanLogError("realloc of fallback UI vertex buffer failed!");

					return false;
				}
				buffers.ui.fallbackVertices = newVertices;

				uint32_t *newIndices = realloc(buffers.ui.fallbackVertices,
											   sizeof(UiVertex) * buffers.ui.fallbackMaxQuads * 4);
				if (!newIndices)
				{
					free(newVertices);
					free(buffers.ui.fallbackVertices);

					free(newIndices);
					free(buffers.ui.fallbackIndices);

					VulkanLogError("realloc of fallback UI index buffer failed!");

					return false;
				}
				buffers.ui.fallbackIndices = newIndices;
			} else
			{
				buffers.ui.fallbackMaxQuads = buffers.ui.maxQuads + 16;

				buffers.ui.fallbackVertices = malloc(sizeof(UiVertex) * buffers.ui.fallbackMaxQuads * 4);
				if (!buffers.ui.fallbackVertices)
				{
					VulkanLogError("malloc of fallback UI vertex buffer failed!");
					return false;
				}
				memcpy(buffers.ui.fallbackVertices, buffers.ui.vertices, sizeof(UiVertex) * buffers.ui.maxQuads * 4);

				buffers.ui.fallbackIndices = malloc(sizeof(uint32_t) * buffers.ui.fallbackMaxQuads * 6);
				if (!buffers.ui.fallbackIndices)
				{
					VulkanLogError("malloc of fallback UI index buffer failed!");

					return false;
				}
				memcpy(buffers.ui.fallbackIndices, buffers.ui.indices, sizeof(uint32_t) * buffers.ui.maxQuads * 6);
			}
		}

		buffers.ui.fallbackVertices[4 * buffers.ui.quadCount] = (UiVertex){
			{
				vertices_posXY_uvZW[0][0],
				vertices_posXY_uvZW[0][1],
				vertices_posXY_uvZW[0][2],
				vertices_posXY_uvZW[0][3],
			},
			{r, g, b, a},
			textureIndex,
		};
		buffers.ui.fallbackVertices[4 * buffers.ui.quadCount + 1] = (UiVertex){
			{
				vertices_posXY_uvZW[1][0],
				vertices_posXY_uvZW[1][1],
				vertices_posXY_uvZW[1][2],
				vertices_posXY_uvZW[1][3],
			},
			{r, g, b, a},
			textureIndex,
		};
		buffers.ui.fallbackVertices[4 * buffers.ui.quadCount + 2] = (UiVertex){
			{
				vertices_posXY_uvZW[2][0],
				vertices_posXY_uvZW[2][1],
				vertices_posXY_uvZW[2][2],
				vertices_posXY_uvZW[2][3],
			},
			{r, g, b, a},
			textureIndex,
		};
		buffers.ui.fallbackVertices[4 * buffers.ui.quadCount + 3] = (UiVertex){
			{
				vertices_posXY_uvZW[3][0],
				vertices_posXY_uvZW[3][1],
				vertices_posXY_uvZW[3][2],
				vertices_posXY_uvZW[3][3],
			},
			{r, g, b, a},
			textureIndex,
		};

		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount] = buffers.ui.quadCount * 4;
		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount + 1] = buffers.ui.quadCount * 4 + 1;
		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount + 2] = buffers.ui.quadCount * 4 + 2;
		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount + 3] = buffers.ui.quadCount * 4;
		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount + 4] = buffers.ui.quadCount * 4 + 2;
		buffers.ui.fallbackIndices[6 * buffers.ui.quadCount + 5] = buffers.ui.quadCount * 4 + 3;

		buffers.ui.quadCount++;

		return true;
	}

	buffers.ui.vertices[4 * buffers.ui.quadCount] = (UiVertex){
		{
			vertices_posXY_uvZW[0][0],
			vertices_posXY_uvZW[0][1],
			vertices_posXY_uvZW[0][2],
			vertices_posXY_uvZW[0][3],
		},
		{r, g, b, a},
		textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 1] = (UiVertex){
		{
			vertices_posXY_uvZW[1][0],
			vertices_posXY_uvZW[1][1],
			vertices_posXY_uvZW[1][2],
			vertices_posXY_uvZW[1][3],
		},
		{r, g, b, a},
		textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 2] = (UiVertex){
		{
			vertices_posXY_uvZW[2][0],
			vertices_posXY_uvZW[2][1],
			vertices_posXY_uvZW[2][2],
			vertices_posXY_uvZW[2][3],
		},
		{r, g, b, a},
		textureIndex,
	};
	buffers.ui.vertices[4 * buffers.ui.quadCount + 3] = (UiVertex){
		{
			vertices_posXY_uvZW[3][0],
			vertices_posXY_uvZW[3][1],
			vertices_posXY_uvZW[3][2],
			vertices_posXY_uvZW[3][3],
		},
		{r, g, b, a},
		textureIndex,
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
