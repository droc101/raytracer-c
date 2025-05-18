// Created by Noah on 11/23/2024.
//

#include "VulkanInternal.h"
#include <dirent.h>
#include <SDL_vulkan.h>
#include <string.h>
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../../Core/MathEx.h"
#include "VulkanHelpers.h"
#include "VulkanMemory.h"
#include "VulkanResources.h"

bool CreateInstance()
{
	uint32_t extensionCount;
	if (SDL_Vulkan_GetInstanceExtensions(vulkanWindow, &extensionCount, NULL) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!\n");
		return false;
	}
	const char *extensionNames[extensionCount];
	if (SDL_Vulkan_GetInstanceExtensions(vulkanWindow, &extensionCount, extensionNames) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!\n");
		return false;
	}
	VkApplicationInfo applicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = GAME_TITLE,
		.applicationVersion = VULKAN_VERSION,
		.pEngineName = GAME_TITLE,
		.engineVersion = VULKAN_VERSION,
		.apiVersion = VK_API_VERSION_1_2,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledExtensionCount = extensionCount,
		.ppEnabledExtensionNames = extensionNames,
	};

#if defined(VK_ENABLE_VALIDATION_LAYER) || defined(VK_ENABLE_MESA_FPS_OVERLAY)
	uint32_t layerCount;
	VulkanTest(vkEnumerateInstanceLayerProperties(&layerCount, NULL), "Failed to enumerate Vulkan instance layers!");
	VkLayerProperties availableLayers[layerCount];
	VulkanTest(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers),
			   "Failed to enumerate Vulkan instance layers!");
#endif
#if defined(VK_ENABLE_VALIDATION_LAYER) && defined(VK_ENABLE_MESA_FPS_OVERLAY)
	uint8_t found = 0;
	for (uint32_t i = 0; i < layerCount; i++)
	{
		if (!strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 27))
		{
			found |= 1;
		}
		if (!strncmp(availableLayers[i].layerName, "VK_LAYER_MESA_overlay", 21))
		{
			found |= 2;
		}
		if (found == 3)
		{
			break;
		}
	}
	if (found != 3)
	{
		if (found == 1)
		{
			FriendlyError("Missing Vulkan Mesa layers!",
						  "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay. "
						  "If you wish to disable the Mesa FPS overlay, that can be done by removing the definition "
						  "for VK_ENABLE_MESA_FPS_OVERLAY in config.h");
		}
		FriendlyError("Missing Vulkan validation layers!",
					  "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\n"
					  "You can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package "
					  "manager of your choice.\n"
					  "If you wish to disable the validation layer, that can be done by removing the definition for "
					  "VK_ENABLE_VALIDATION_LAYER in config.h");
	}
	createInfo.enabledLayerCount = 2;
	createInfo.ppEnabledLayerNames = (const char *const[2]){"VK_LAYER_KHRONOS_validation", "VK_LAYER_MESA_overlay"};
#elifdef VK_ENABLE_VALIDATION_LAYER
	bool found = false;
	for (uint32_t i = 0; i < layerCount; i++)
	{
		if (!strncmp(availableLayers[i].layerName, "VK_LAYER_KHRONOS_validation", 27))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		FriendlyError("Missing Vulkan validation layers!",
					  "The Vulkan SDK must be installed on your device to use the Vulkan validation layer.\n"
					  "You can get the Vulkan SDK from https://vulkan.lunarg.com/sdk/home or by using the package "
					  "manager of your choice.\n"
					  "If you wish to disable the validation layer, that can be done by removing the definition for "
					  "VK_ENABLE_VALIDATION_LAYER in config.h");
	}
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#elifdef VK_ENABLE_MESA_FPS_OVERLAY
	bool found = false;
	for (uint32_t i = 0; i < layerCount; i++)
	{
		if (!strncmp(availableLayers[i].layerName, "VK_LAYER_MESA_overlay", 21))
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		FriendlyError("Missing Vulkan Mesa layers!",
					  "The Vulkan Mesa layers must be installed on your device to use the Mesa FPS overlay.\n"
					  "If you wish to disable the Mesa FPS overlay, that can be done by removing the definition for "
					  "VK_ENABLE_MESA_FPS_OVERLAY in config.h");
	}
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_MESA_overlay"};
#endif

	VulkanTest(vkCreateInstance(&createInfo, NULL, &instance), "Failed to create Vulkan instance!");

	return true;
}

bool CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(vulkanWindow, instance, &surface) == SDL_FALSE)
	{
		VulkanLogError("Failed to create Vulkan window surface\n");
		return false;
	}

	return true;
}

bool PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	VulkanTest(vkEnumeratePhysicalDevices(instance, &deviceCount, NULL), "Failed to enumerate physical devices!");
	if (deviceCount == 0)
	{
		VulkanLogError("Failed to find any GPUs with Vulkan support!\n");
		return false;
	}
	VkPhysicalDevice devices[deviceCount];
	VulkanTest(vkEnumeratePhysicalDevices(instance, &deviceCount, devices), "Failed to enumerate physical devices!");

	bool match = false;
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		queueFamilyIndices = (QueueFamilyIndices){
			.graphicsFamily = -1,
			.presentFamily = -1,
			.uniquePresentFamily = -1,
			.transferFamily = -1,
			.families = QUEUE_FAMILY_GRAPHICS,
			.familyCount = 1,
		};
		const VkPhysicalDevice pDevice = devices[i];

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
		if (!deviceFeatures.geometryShader || !deviceFeatures.samplerAnisotropy)
		{
			continue;
		}
		if (!QuerySwapChainSupport(pDevice))
		{
			VulkanLogError("Failed to query swap chain support!\n");
			return false;
		}
		if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0)
		{
			continue;
		}

		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, NULL);
		VkQueueFamilyProperties families[familyCount];
		vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, families);
		for (uint32_t index = 0; index < familyCount;
			 index++) // TODO: This does not necessarily find the most optimal presentation family
		{
			VkBool32 presentSupport = VK_FALSE;
			VulkanTest(vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, index, surface, &presentSupport),
					   "Failed to check device for presentation support!");

			if (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueFamilyIndices.graphicsFamily = index;
				if (presentSupport)
				{
					queueFamilyIndices.presentFamily = index;
				}
			} else
			{
				if (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					queueFamilyIndices.transferFamily = index;
				}
				if (presentSupport)
				{
					queueFamilyIndices.uniquePresentFamily = index;
				}
			}

			if ((queueFamilyIndices.presentFamily == -1 && queueFamilyIndices.uniquePresentFamily == -1) ||
				queueFamilyIndices.graphicsFamily == -1 ||
				queueFamilyIndices.transferFamily == -1)
			{
				continue;
			}

			break;
		}

		if (queueFamilyIndices.presentFamily == -1)
		{
			queueFamilyIndices.presentFamily = queueFamilyIndices.uniquePresentFamily;
		}
		if (queueFamilyIndices.transferFamily == -1)
		{
			queueFamilyIndices.transferFamily = queueFamilyIndices.graphicsFamily;
		}

		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
		{
			queueFamilyIndices.families |= QUEUE_FAMILY_PRESENTATION;
			queueFamilyIndices.familyCount++;
		}
		if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.transferFamily)
		{
			queueFamilyIndices.families |= QUEUE_FAMILY_TRANSFER;
			queueFamilyIndices.familyCount++;
		}

		uint32_t extensionCount;
		VulkanTest(vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL),
				   "Failed to enumerate device extensions!");
		if (extensionCount == 0)
		{
			continue;
		}
		VkExtensionProperties availableExtensions[extensionCount];
		VulkanTest(vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, availableExtensions),
				   "Failed to enumerate Vulkan device extensions!");
		for (uint32_t j = 0; j < extensionCount; j++)
		{
			if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				VkPhysicalDeviceProperties deviceProperties;
				VkPhysicalDeviceMemoryProperties memoryProperties;

				vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
				vkGetPhysicalDeviceMemoryProperties(devices[i], &memoryProperties);

				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					physicalDevice.device = devices[i];
					physicalDevice.features = deviceFeatures;
					physicalDevice.properties = deviceProperties;
					physicalDevice.memoryProperties = memoryProperties;
					return true;
				}
				physicalDevice.device = devices[i];
				physicalDevice.features = deviceFeatures;
				physicalDevice.properties = deviceProperties;
				physicalDevice.memoryProperties = memoryProperties;
				match = true;
				break;
			}
		}
	}

	if (!match)
	{
		VulkanLogError("Failed to find a suitable GPU for Vulkan!\n");
	}

	return match;
}

bool CreateLogicalDevice()
{
	const float queuePriority = 1;

	VkDeviceQueueCreateInfo queueCreateInfos[queueFamilyIndices.familyCount];
	switch (queueFamilyIndices.familyCount)
	{
		case 3:
			queueCreateInfos[2] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamilyIndices.presentFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 2:
			queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											? queueFamilyIndices.transferFamily
											: queueFamilyIndices.presentFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 1:
			queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamilyIndices.graphicsFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
			break;
		default:
			VulkanLogError("Failed to create VkDeviceQueueCreateInfo due to invalid queueFamilyIndices!\n");
			return false;
	}

	VkPhysicalDeviceFeatures deviceFeatures = {
		.logicOp = VK_TRUE,
		.multiDrawIndirect = VK_TRUE,
		.samplerAnisotropy = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features vulkan12Features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.runtimeDescriptorArray = VK_TRUE,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
		.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
	};
	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &vulkan12Features,
		.queueCreateInfoCount = queueFamilyIndices.familyCount,
		.pQueueCreateInfos = queueCreateInfos,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = (const char *const[1]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.pEnabledFeatures = &deviceFeatures,
	};

#ifdef VK_ENABLE_VALIDATION_LAYER
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#endif

	VulkanTest(vkCreateDevice(physicalDevice.device, &createInfo, NULL, &device), "Failed to create Vulkan device!");

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.transferFamily, 0, &transferQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);

	return true;
}

bool CreateSwapChain()
{
	if (minimized)
	{
		return true;
	}

	if (!QuerySwapChainSupport(physicalDevice.device))
	{
		VulkanLogError("Failed to query Vulkan swap chain support!\n");
		return false;
	}

	if (!swapChainSupport.capabilities.currentExtent.width || !swapChainSupport.capabilities.currentExtent.height)
	{
		// Window is minimized, so return to prevent creating a swap chain with dimensions of 0px by 0px
		// However, we do not want to fail or even log anything, since this is intended behavior
		minimized = true;
		return true;
	}

	VkSurfaceFormatKHR surfaceFormat = {.format = VK_FORMAT_MAX_ENUM, .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR};
	for (uint32_t i = 0; i < swapChainSupport.formatCount; i++)
	{
		const VkSurfaceFormatKHR format = swapChainSupport.formats[i];
		if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM || surfaceFormat.format == VK_FORMAT_MAX_ENUM)
			{
				surfaceFormat = format;
			} else if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				surfaceFormat = format;
				break;
			}
		}
	}
	if (surfaceFormat.format == VK_FORMAT_MAX_ENUM || surfaceFormat.colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
	{
		VulkanLogError("Unable to find suitable Vulkan swap chain color format!\n");
		return false;
	}

	VkExtent2D extent = swapChainSupport.capabilities.currentExtent;
	if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
	{
		int32_t width;
		int32_t height;
		SDL_Vulkan_GetDrawableSize(vulkanWindow, &width, &height);
		extent.width = clamp(width,
							 swapChainSupport.capabilities.minImageExtent.width,
							 swapChainSupport.capabilities.maxImageExtent.width);
		extent.height = clamp(height,
							  swapChainSupport.capabilities.minImageExtent.height,
							  swapChainSupport.capabilities.maxImageExtent.height);
	}
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

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

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	if (!GetState()->options.vsync)
	{
		for (uint32_t i = 0; i < swapChainSupport.presentModeCount; i++)
		{
			const VkPresentModeKHR mode = swapChainSupport.presentMode[i];
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}
	if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
	{
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	const VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = queueFamilyIndices.families & QUEUE_FAMILY_PRESENTATION ? VK_SHARING_MODE_CONCURRENT
																					: VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = queueFamilyIndices.familyCount,
		.pQueueFamilyIndices = pQueueFamilyIndices,
		.preTransform = swapChainSupport.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};
	VulkanTest(vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain), "Failed to create Vulkan swap chain!");

	VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL), "Failed to get Vulkan swapchain images!");
	swapChainImages = malloc(sizeof(*swapChainImages) * imageCount);
	CheckAlloc(swapChainImages);
	swapChainCount = imageCount;
	VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages),
			   "Failed to get Vulkan swapchain images!");
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	return true;
}

bool CreateImageViews()
{
	swapChainImageViews = malloc(sizeof(*swapChainImageViews) * swapChainCount);
	CheckAlloc(swapChainImageViews);

	for (uint32_t i = 0; i < swapChainCount; i++)
	{
		if (!CreateImageView(&swapChainImageViews[i],
							 swapChainImages[i],
							 swapChainImageFormat,
							 VK_IMAGE_ASPECT_COLOR_BIT,
							 1,
							 "Failed to create Vulkan swap chain image view!"))
		{
			return false;
		}
	}

	return true;
}

bool CreateRenderPass()
{
	// TODO if stencil is not needed then allow for using VK_FORMAT_D16_UNORM
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice.device, VK_FORMAT_D24_UNORM_S8_UINT, &properties);
	if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		depthImageFormat = VK_FORMAT_D24_UNORM_S8_UINT;
	} else
	{
		vkGetPhysicalDeviceFormatProperties(physicalDevice.device, VK_FORMAT_D32_SFLOAT_S8_UINT, &properties);
		if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthImageFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		} else
		{
			VulkanLogError("Unable to find suitable format for Vulkan depth image!\n");
			return false;
		}
	}

	switch (GetState()->options.msaa)
	{
		case MSAA_2X:
			msaaSamples = VK_SAMPLE_COUNT_2_BIT;
			break;
		case MSAA_4X:
			msaaSamples = VK_SAMPLE_COUNT_4_BIT;
			break;
		case MSAA_8X:
			msaaSamples = VK_SAMPLE_COUNT_8_BIT;
			break;
		case MSAA_NONE:
		default:
			msaaSamples = VK_SAMPLE_COUNT_1_BIT;
			break;
	}
	if (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
		  physicalDevice.properties.limits.framebufferDepthSampleCounts &
		  msaaSamples))
	{
		ShowWarning("Invalid Settings",
					"Your GPU driver does not support the selected MSAA level!\n"
					"A fallback has been set to avoid issues.");
		while (!(physicalDevice.properties.limits.framebufferColorSampleCounts &
				 physicalDevice.properties.limits.framebufferDepthSampleCounts &
				 msaaSamples))
		{
			msaaSamples >>= 1;
		}
	}

	const VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentReference depthAttachmentReference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	const VkSubpassDependency dependencies[2] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
		{
			.srcSubpass = 0,
			.dstSubpass = 1,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	};

	if (msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		const VkAttachmentDescription colorAttachment = {
			.format = swapChainImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		const VkAttachmentDescription depthAttachment = {
			.format = depthImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};

		const VkSubpassDescription wallSubpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pDepthStencilAttachment = &depthAttachmentReference,
		};
		const VkSubpassDescription uiSubpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
		};

		const VkRenderPassCreateInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 2,
			.pAttachments = (VkAttachmentDescription[]){colorAttachment, depthAttachment},
			.subpassCount = 2,
			.pSubpasses = (VkSubpassDescription[]){wallSubpass, uiSubpass},
			.dependencyCount = 2,
			.pDependencies = dependencies,
		};

		VulkanTest(vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass),
				   "Failed to create Vulkan render pass!");
	} else
	{
		const VkAttachmentDescription colorAttachment = {
			.format = swapChainImageFormat,
			.samples = msaaSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		const VkAttachmentDescription depthAttachment = {
			.format = depthImageFormat,
			.samples = msaaSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		};
		const VkAttachmentDescription colorResolveAttachment = {
			.format = swapChainImageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};

		const VkAttachmentReference colorAttachmentResolveRef = {
			.attachment = 2,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};
		const VkSubpassDescription wallSubpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = &colorAttachmentResolveRef,
			.pDepthStencilAttachment = &depthAttachmentReference,
		};
		const VkSubpassDescription uiSubpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = &colorAttachmentResolveRef,
		};

		const VkRenderPassCreateInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 3,
			.pAttachments = (VkAttachmentDescription[]){colorAttachment, depthAttachment, colorResolveAttachment},
			.subpassCount = 2,
			.pSubpasses = (VkSubpassDescription[]){wallSubpass, uiSubpass},
			.dependencyCount = 2,
			.pDependencies = dependencies,
		};

		VulkanTest(vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass),
				   "Failed to create Vulkan render pass!");
	}

	return true;
}

bool CreateDescriptorSetLayouts()
{
	const VkDescriptorSetLayoutBinding binding = {
		.binding = 0,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = MAX_TEXTURES,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	};
	const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = 1,
		.pBindingFlags = (VkDescriptorBindingFlags[]){VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT},
	};
	const VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &bindingFlagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = 1,
		.pBindings = &binding,
	};

	VulkanTest(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout),
			   "Failed to create pipeline descriptor set layout!");

	return true;
}

bool CreateGraphicsPipelineCache()
{
	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
	};
	VulkanTest(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL, &pipelineCache),
			   "Failed to create graphics pipeline cache!");

	return true;
}

bool CreateGraphicsPipelines()
{
#pragma region shared
	const VkViewport viewport = {
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.maxDepth = 1,
	};
	const VkRect2D scissor = {
		.extent = swapChainExtent,
	};
	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = msaaSamples,
		.minSampleShading = 1,
	};

	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.maxDepthBounds = 1,
	};

	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};
	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
	};

	if (sizeof(PushConstants) > physicalDevice.properties.limits.maxPushConstantsSize)
	{
		Error("Go support core 1.0 then get back to me. (Max push constant size exceeded)");
	}
	VkPushConstantRange pushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = sizeof(PushConstants),
	};
	const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange,
	};
	VulkanTest(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout),
			   "Failed to create graphics pipeline layout!");

	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};
#pragma endregion shared

#pragma region walls
	const VkShaderModule wallVertShaderModule = CreateShaderModule(VK_VERT("Vulkan_wall"));
	const VkShaderModule wallFragShaderModule = CreateShaderModule(VK_FRAG("Vulkan_wall"));
	if (!wallVertShaderModule || !wallFragShaderModule)
	{
		VulkanLogError("Failed to load wall shaders!\n");
		return false;
	}

	const VkPipelineShaderStageCreateInfo wallShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = wallVertShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = wallFragShaderModule,
			.pName = "main",
		},
	};

	const VkVertexInputBindingDescription wallBindingDescriptions[2] = {
		{
			.binding = 0,
			.stride = sizeof(ShadowVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(WallVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription wallVertexDescriptions[5] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ShadowVertex, x),
		},
		{
			.location = 1,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(WallVertex, x),
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(WallVertex, u),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(WallVertex, textureIndex),
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(WallVertex, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo wallVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = wallBindingDescriptions,
		.vertexAttributeDescriptionCount = 5,
		.pVertexAttributeDescriptions = wallVertexDescriptions,
	};

	VkGraphicsPipelineCreateInfo wallPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = wallShaderStages,
		.pVertexInputState = &wallVertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.basePipelineIndex = -1,
	};
#pragma endregion walls

#pragma region actors
	const VkShaderModule actorVertShaderModule = CreateShaderModule(VK_VERT("Vulkan_actor"));
	const VkShaderModule actorFragShaderModule = CreateShaderModule(VK_FRAG("Vulkan_actor"));
	if (!actorVertShaderModule || !actorFragShaderModule)
	{
		VulkanLogError("Failed to load actor shaders!\n");
		return false;
	}

	const VkPipelineShaderStageCreateInfo actorShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = actorVertShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = actorFragShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
	};

	const VkVertexInputBindingDescription actorBindingDescriptions[2] = {
		{
			.binding = 0,
			.stride = sizeof(ActorVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
		{
			.binding = 1,
			.stride = sizeof(ActorInstanceData),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
		},
	};
	const VkVertexInputAttributeDescription actorVertexDescriptions[9] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(ActorVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(ActorVertex, nx),
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 0,
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 1,
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 2,
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(ActorInstanceData, transform) + sizeof(vec4) * 3,
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(ActorInstanceData, textureIndex),
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(ActorInstanceData, wallAngle),
		},
	};
	const VkPipelineVertexInputStateCreateInfo actorVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 2,
		.pVertexBindingDescriptions = actorBindingDescriptions,
		.vertexAttributeDescriptionCount = 9,
		.pVertexAttributeDescriptions = actorVertexDescriptions,
	};

	VkGraphicsPipelineCreateInfo actorPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = actorShaderStages,
		.pVertexInputState = &actorVertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.basePipelineIndex = -1,
	};
#pragma endregion actors

#pragma region UI
	const VkShaderModule uiVertShaderModule = CreateShaderModule(VK_VERT("Vulkan_ui"));
	const VkShaderModule uiFragShaderModule = CreateShaderModule(VK_FRAG("Vulkan_ui"));
	if (!uiVertShaderModule || !uiFragShaderModule)
	{
		VulkanLogError("Failed to load colored quad shaders!\n");
		return false;
	}

	const VkPipelineShaderStageCreateInfo uiShaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = uiVertShaderModule,
			.pName = "main",
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = uiFragShaderModule,
			.pName = "main",
		},
	};

	const VkVertexInputBindingDescription uiBindingDescription = {
		.binding = 0,
		.stride = sizeof(UiVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription uiAttributeDescriptions[4] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(UiVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(UiVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(UiVertex, r),
		},
		{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(UiVertex, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo uiVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &uiBindingDescription,
		.vertexAttributeDescriptionCount = 4,
		.pVertexAttributeDescriptions = uiAttributeDescriptions,
	};

	VkGraphicsPipelineCreateInfo uiPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = uiShaderStages,
		.pVertexInputState = &uiVertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 1,
		.basePipelineIndex = -1,
	};
#pragma endregion UI


	VkGraphicsPipelineCreateInfo pipelinesCreateInfo[3] = {
		wallPipelineInfo,
		actorPipelineInfo,
		uiPipelineInfo,
	};
	VkPipeline pipelineList[3] = {0};

	VulkanTest(vkCreateGraphicsPipelines(device, pipelineCache, 3, pipelinesCreateInfo, NULL, pipelineList),
			   "Failed to create graphics pipelines!");

	pipelines.walls = pipelineList[0];
	pipelines.actors = pipelineList[1];
	pipelines.ui = pipelineList[2];


	vkDestroyShaderModule(device, wallVertShaderModule, NULL);
	vkDestroyShaderModule(device, wallFragShaderModule, NULL);

	vkDestroyShaderModule(device, actorVertShaderModule, NULL);
	vkDestroyShaderModule(device, actorFragShaderModule, NULL);

	vkDestroyShaderModule(device, uiVertShaderModule, NULL);
	vkDestroyShaderModule(device, uiFragShaderModule, NULL);

	return true;
}

bool CreateCommandPools()
{
	const VkCommandPoolCreateInfo graphicsPoolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndices.graphicsFamily,
	};

	VulkanTest(vkCreateCommandPool(device, &graphicsPoolInfo, NULL, &graphicsCommandPool),
			   "Failed to create graphics command pool!");

	const VkCommandPoolCreateInfo transferPoolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queueFamilyIndices.transferFamily,
	};

	VulkanTest(vkCreateCommandPool(device, &transferPoolInfo, NULL, &transferCommandPool),
			   "Failed to create transfer command pool!");

	return true;
}

bool CreateColorImage()
{
	if (msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		return true;
	}
	if (!CreateImage(&colorImage,
					 &colorImageMemory,
					 swapChainImageFormat,
					 (VkExtent3D){.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
					 1,
					 msaaSamples,
					 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
					 "color"))
	{
		return false;
	}

	if (!CreateImageView(&colorImageView,
						 colorImage,
						 swapChainImageFormat,
						 VK_IMAGE_ASPECT_COLOR_BIT,
						 1,
						 "Failed to create Vulkan color image view!"))
	{
		return false;
	}

	return true;
}

bool CreateDepthImage()
{
	if (!CreateImage(&depthImage,
					 &depthImageMemory,
					 depthImageFormat,
					 (VkExtent3D){.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
					 1,
					 msaaSamples,
					 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
					 "depth test"))
	{
		return false;
	}

	if (!CreateImageView(&depthImageView,
						 depthImage,
						 depthImageFormat,
						 VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
						 1,
						 "Failed to create Vulkan depth image view!"))
	{
		return false;
	}

	return true;
}

bool CreateFramebuffers()
{
	swapChainFramebuffers = malloc(sizeof(*swapChainFramebuffers) * swapChainCount);
	CheckAlloc(swapChainFramebuffers);

	for (uint32_t i = 0; i < swapChainCount; i++)
	{
		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass,
			.attachmentCount = msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3,
			.pAttachments = msaaSamples == VK_SAMPLE_COUNT_1_BIT
									? (VkImageView[2]){swapChainImageViews[i], depthImageView}
									: (VkImageView[3]){colorImageView, depthImageView, swapChainImageViews[i]},
			.width = swapChainExtent.width,
			.height = swapChainExtent.height,
			.layers = 1,
		};

		VulkanTest(vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]),
				   "Failed to create Vulkan framebuffer!");
	}

	return true;
}

bool InitTextures()
{
	const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice.device, format, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		VulkanLogError("Vulkan texture image format does not support linear blitting!\n");
		return false;
	}

	ListCreate(&textures);
	memset(imageAssetIdToIndexMap, -1, sizeof(*imageAssetIdToIndexMap) * MAX_TEXTURES);

	char texturesPath[300];
	sprintf(texturesPath, "%sassets/texture/", GetState()->executableFolder);
	DIR *dir = opendir(texturesPath);
	if (dir == NULL)
	{
		VulkanLogError("Failed to open texture directory: %s\n", texturesPath);
		textureCount = 1;
	} else
	{
		const struct dirent *ent = readdir(dir);
		while (ent != NULL)
		{
			if (strstr(ent->d_name, ".gtex") != NULL)
			{
				textureCount++;
			}
			ent = readdir(dir);
		}
		closedir(dir);
		if (textureCount == 0)
		{
			VulkanLogError("Failed to find any valid textures!");
			textureCount = 1;
		}
	}

	VkImage image;
	if (!CreateImage(&image,
					 NULL,
					 format,
					 (VkExtent3D){1, 1, 1},
					 1,
					 VK_SAMPLE_COUNT_1_BIT,
					 VK_IMAGE_USAGE_SAMPLED_BIT,
					 ""))
	{
		return false;
	}
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);
	textureMemory.size = memoryRequirements.alignment *
						 (VkDeviceSize)ceil((double)(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4) /
											(double)memoryRequirements.alignment) *
						 textureCount;
	for (uint32_t i = 0; i < physicalDevice.memoryProperties.memoryTypeCount; i++)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i &&
			(physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & textureMemory.type) == textureMemory.type)
		{
			const VkMemoryAllocateInfo allocateInfo = {
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = textureMemory.size,
				.memoryTypeIndex = i,
			};

			VulkanTest(vkAllocateMemory(device, &allocateInfo, NULL, &textureMemory.memory),
					   "Failed to allocate Vulkan texture memory!");
			break;
		}
	}

	vkDestroyImage(device, image, NULL);

	return true;
}

bool CreateTexturesImageView()
{
	ListCreate(&texturesImageView);
	for (size_t textureIndex = 0; textureIndex < textures.length; textureIndex++)
	{
		const Texture *texture = ListGet(textures, textureIndex);
		VkImageView *textureImageView = malloc(sizeof(VkImageView *));
		CheckAlloc(textureImageView);
		ListAdd(&texturesImageView, textureImageView);
		if (!CreateImageView(textureImageView,
							 texture->image,
							 VK_FORMAT_R8G8B8A8_UNORM,
							 VK_IMAGE_ASPECT_COLOR_BIT,
							 texture->mipmapLevels,
							 "Failed to create Vulkan texture image view!"))
		{
			return false;
		}
	}

	return true;
}

bool CreateTextureSampler()
{
	const VkSamplerCreateInfo linearRepeatSamplerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = -1.5f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	const VkSamplerCreateInfo nearestRepeatSamplerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = -1.5f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	const VkSamplerCreateInfo linearNoRepeatSamplerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};
	const VkSamplerCreateInfo nearestNoRepeatSamplerCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.mipLodBias = -1.5f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 1,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_ALWAYS,
		.minLod = 0,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		.unnormalizedCoordinates = VK_FALSE,
	};

	VulkanTest(vkCreateSampler(device, &linearRepeatSamplerCreateInfo, NULL, &textureSamplers.linearRepeat),
			   "Failed to create linear repeating texture sampler!");
	VulkanTest(vkCreateSampler(device, &nearestRepeatSamplerCreateInfo, NULL, &textureSamplers.nearestRepeat),
			   "Failed to create nearest repeating texture sampler!");
	VulkanTest(vkCreateSampler(device, &linearNoRepeatSamplerCreateInfo, NULL, &textureSamplers.linearNoRepeat),
			   "Failed to create linear non-repeating texture sampler!");
	VulkanTest(vkCreateSampler(device, &nearestNoRepeatSamplerCreateInfo, NULL, &textureSamplers.nearestNoRepeat),
			   "Failed to create nearest non-repeating texture sampler!");

	return true;
}

bool CreateBuffers()
{
	buffers.walls.maxWallCount = MAX_WALLS_INIT;
	buffers.ui.maxQuads = MAX_UI_QUADS_INIT;
	if (!CreateLocalBuffer())
	{
		return false;
	}
	SetLocalBufferAliasingInfo();

	if (!CreateSharedBuffer())
	{
		return false;
	}
	SetSharedBufferAliasingInfo();

	const MemoryAllocationInfo allocationInfo = {
		.memoryInfo = &memoryPools.stagingMemory,
		.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	};
	buffers.staging.memoryAllocationInfo = allocationInfo;
	buffers.staging.size = 33554432;
	if (!CreateBuffer(&buffers.staging, true))
	{
		return false;
	}
	VulkanTest(vkMapMemory(device,
						   buffers.staging.memoryAllocationInfo.memoryInfo->memory,
						   0,
						   VK_WHOLE_SIZE,
						   0,
						   &buffers.staging.memoryAllocationInfo.memoryInfo->mappedMemory),
			   "Failed to map actor model staging buffer memory!");


	return true;
}

bool AllocateMemoryPools()
{
	if (!CreateLocalMemory())
	{
		return false;
	}
	if (!CreateSharedMemory())
	{
		return false;
	}

	return true;
}

bool CreateDescriptorPool()
{
	const VkDescriptorPoolSize poolSizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
		},
	};
	const VkDescriptorPoolCreateInfo poolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = 1,
		.pPoolSizes = poolSizes,
	};

	VulkanTest(vkCreateDescriptorPool(device, &poolCreateInfo, NULL, &descriptorPool),
			   "Failed to create descriptor pool!");

	return true;
}

bool CreateDescriptorSets()
{
	VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		layouts[i] = descriptorSetLayout;
	}

	const VkDescriptorSetAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.pSetLayouts = layouts,
	};

	VulkanTest(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets),
			   "Failed to allocate Vulkan descriptor sets!");

	return true;
}

bool CreateCommandBuffers()
{
	const VkCommandBufferAllocateInfo graphicsAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = graphicsCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};
	VulkanTest(vkAllocateCommandBuffers(device, &graphicsAllocateInfo, commandBuffers),
			   "Failed to allocate Vulkan graphics command buffers!");

	const VkCommandBufferAllocateInfo transferAllocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = transferCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	VulkanTest(vkAllocateCommandBuffers(device, &transferAllocateInfo, &transferCommandBuffer),
			   "Failed to allocate Vulkan transfer command buffer!");

	const VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	VulkanTest(vkCreateFence(device, &fenceCreateInfo, NULL, &transferBufferFence),
			   "Failed to create Vulkan transfer buffer fence!");

	return true;
}

bool CreateSyncObjects()
{
	const VkSemaphoreCreateInfo semaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	const VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]),
				   "Failed to create Vulkan semaphores!");

		VulkanTest(vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]),
				   "Failed to create Vulkan semaphores!");

		VulkanTest(vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]), "Failed to create Vulkan semaphores!");
	}

	return true;
}
