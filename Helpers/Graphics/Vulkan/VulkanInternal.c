//
// Created by Noah on 11/23/2024.
//

#include "VulkanInternal.h"

#include <dirent.h>
#include <SDL_vulkan.h>
#include <string.h>
#include "../../../Structs/GlobalState.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"
#include "VulkanHelpers.h"
#include "VulkanMemory.h"
#include "VulkanResources.h"

bool CreateInstance()
{
	uint32_t extensionCount;
	if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, NULL) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!\n");
		return false;
	}
	const char *extensionNames[extensionCount];
	if (SDL_Vulkan_GetInstanceExtensions(vk_window, &extensionCount, extensionNames) == SDL_FALSE)
	{
		VulkanLogError("Failed to acquire Vulkan extensions required for SDL window!\n");
		return false;
	}
	VkApplicationInfo applicationInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = GAME_TITLE,
		.applicationVersion = VULKAN_VERSION,
		.pEngineName = GAME_TITLE,
		.engineVersion = VULKAN_VERSION,
		.apiVersion = VK_API_VERSION_1_3,
	};
	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
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

	VulkanTest(vkCreateInstance(&createInfo, NULL, &instance), "Failed to create Vulkan instance!")

			return true;
}

bool CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(vk_window, instance, &surface) == SDL_FALSE)
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
			if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
			{
				if (!QuerySwapChainSupport(pDevice))
				{
					VulkanLogError("Failed to query swap chain support!\n");
					return false;
				}
				if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0)
				{
					continue;
				}

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
				.pNext = NULL,
				.flags = 0,
				.queueFamilyIndex = queueFamilyIndices.presentFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 2:
			queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.queueFamilyIndex = queueFamilyIndices.families & QUEUE_FAMILY_TRANSFER
											? queueFamilyIndices.transferFamily
											: queueFamilyIndices.presentFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
		case 1:
			queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
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
		.samplerAnisotropy = VK_TRUE,
		.tessellationShader = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features vulkan12Features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = NULL,
		.runtimeDescriptorArray = VK_TRUE,
		.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
	};
	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &vulkan12Features,
		.flags = 0,
		.queueCreateInfoCount = queueFamilyIndices.familyCount,
		.pQueueCreateInfos = queueCreateInfos,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
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
		SDL_Vulkan_GetDrawableSize(vk_window, &width, &height);
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
		.pNext = NULL,
		.flags = 0,
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
	swapChainImages = (VkImage *)malloc(sizeof(*swapChainImages) * imageCount);
	swapChainCount = imageCount;
	VulkanTest(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages),
			   "Failed to get Vulkan swapchain images!");
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	return true;
}

bool CreateImageViews()
{
	swapChainImageViews = (VkImageView *)malloc(sizeof(*swapChainImageViews) * swapChainCount);

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
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		},
		{
			.srcSubpass = 0,
			.dstSubpass = 1,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = 0,
		},
	};

	if (msaaSamples == VK_SAMPLE_COUNT_1_BIT)
	{
		const VkAttachmentDescription colorAttachment = {
			.flags = 0,
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
			.flags = 0,
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
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = &depthAttachmentReference,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL,
		};
		const VkSubpassDescription uiSubpass = {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = NULL,
			.pDepthStencilAttachment = NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL,
		};

		const VkRenderPassCreateInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
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
			.flags = 0,
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
			.flags = 0,
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
			.flags = 0,
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
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = &colorAttachmentResolveRef,
			.pDepthStencilAttachment = &depthAttachmentReference,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL,
		};
		const VkSubpassDescription uiSubpass = {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = &colorAttachmentResolveRef,
			.pDepthStencilAttachment = NULL,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = NULL,
		};

		const VkRenderPassCreateInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
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
	const VkDescriptorSetLayoutBinding bindings[2] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL,
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL,
		},
	};
	const VkDescriptorSetLayoutCreateInfo layoutInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = 2,
		.pBindings = bindings,
	};

	VulkanTest(vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout),
			   "Failed to create pipeline descriptor set layout!");

	return true;
}

bool CreateGraphicsPipelineCache()
{
	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = NULL,
	};
	VulkanTest(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, NULL, &pipelineCache),
			   "Failed to create graphics pipeline cache!");

	return true;
}

bool CreateGraphicsPipelines()
{
#pragma region shared
	const VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (float)swapChainExtent.width,
		.height = (float)swapChainExtent.height,
		.minDepth = 0,
		.maxDepth = 1,
	};
	const VkRect2D scissor = {
		.offset = {0, 0},
		.extent = swapChainExtent,
	};
	const VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0,
		.depthBiasClamp = 0,
		.depthBiasSlopeFactor = 0,
		.lineWidth = 1,
	};

	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = msaaSamples,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {0},
		.back = {0},
		.minDepthBounds = 0,
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
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = {0, 0, 0, 0},
	};

	const VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = 0,
		.pDynamicStates = NULL,
	};

	VkPushConstantRange pushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.size = 8,
	};
	const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange,
	};
	VulkanTest(vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout),
			   "Failed to create graphics pipeline layout!");
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
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = wallVertShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = wallFragShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
	};

	const VkVertexInputBindingDescription wallBindingDescriptions[1] = {
		{
			.binding = 0,
			.stride = sizeof(WallVertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};
	const VkVertexInputAttributeDescription wallVertexDescriptions[3] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(WallVertex, x),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(WallVertex, u),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(WallVertex, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo wallVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = wallBindingDescriptions,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = wallVertexDescriptions,
	};

	const VkPipelineInputAssemblyStateCreateInfo wallInputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkGraphicsPipelineCreateInfo wallsPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = 2,
		.pStages = wallShaderStages,
		.pVertexInputState = &wallVertexInputInfo,
		.pInputAssemblyState = &wallInputAssembly,
		.pTessellationState = NULL,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
#pragma endregion walls

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
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = uiVertShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = uiFragShaderModule,
			.pName = "main",
			.pSpecializationInfo = NULL,
		},
	};

	const VkVertexInputBindingDescription uiBindingDescription = {
		.binding = 0,
		.stride = sizeof(UiVertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
	const VkVertexInputAttributeDescription uiAttributeDescriptions[3] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(UiVertex, posXY_uvZW),
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(UiVertex, color),
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32_UINT,
			.offset = offsetof(UiVertex, textureIndex),
		},
	};
	const VkPipelineVertexInputStateCreateInfo uiVertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &uiBindingDescription,
		.vertexAttributeDescriptionCount = 3,
		.pVertexAttributeDescriptions = uiAttributeDescriptions,
	};

	const VkPipelineInputAssemblyStateCreateInfo uiInputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	VkGraphicsPipelineCreateInfo uiPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = 2,
		.pStages = uiShaderStages,
		.pVertexInputState = &uiVertexInputInfo,
		.pInputAssemblyState = &uiInputAssembly,
		.pTessellationState = NULL,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = NULL,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 1,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};
#pragma endregion UI


	VkGraphicsPipelineCreateInfo pipelinesCreateInfo[2] = {
		wallsPipelineInfo,
		uiPipelineInfo,
	};
	VkPipeline pipelineList[2] = {0};

	VulkanTest(vkCreateGraphicsPipelines(device, pipelineCache, 2, pipelinesCreateInfo, NULL, pipelineList),
			   "Failed to create graphics pipelines!");

	pipelines.walls = pipelineList[0];
	pipelines.ui = pipelineList[1];


	vkDestroyShaderModule(device, wallVertShaderModule, NULL);
	vkDestroyShaderModule(device, wallFragShaderModule, NULL);

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
	if (!CreateImage(&colorImage,
					 &colorImageMemory,
					 swapChainImageFormat,
					 (VkExtent3D){.width = swapChainExtent.width, .height = swapChainExtent.height, .depth = 1},
					 1,
					 msaaSamples,
					 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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

	const VkCommandBuffer commandBuffer;
	if (!BeginCommandBuffer(&commandBuffer, graphicsCommandPool))
	{
		return false;
	}

	const VkImageSubresourceRange subresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};
	const VkImageMemoryBarrier transferBarrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = depthImage,
		.subresourceRange = subresourceRange,
	};

	vkCmdPipelineBarrier(commandBuffer,
						 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						 0,
						 0,
						 NULL,
						 0,
						 NULL,
						 1,
						 &transferBarrier);

	if (!EndCommandBuffer(commandBuffer, graphicsCommandPool, graphicsQueue))
	{
		return false;
	}

	return true;
}

bool CreateFramebuffers()
{
	swapChainFramebuffers = (VkFramebuffer *)malloc(sizeof(*swapChainFramebuffers) * swapChainCount);

	for (uint32_t i = 0; i < swapChainCount; i++)
	{
		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
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

	ListCreate(&textures, 0);
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

	/// TODO: This will first be written using a seperate allocation from the walls and other things that use VRAM, but
	///  then once the code is fully working and comitted, I will come back to this and add the textures to the shared
	///  allocation to improve performance.
	textureMemory.size = MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * textureCount;

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
	vkDestroyImage(device, image, NULL);
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

	return true;
}

bool CreateTexturesImageView()
{
	ListCreate(&texturesImageView, 0);
	for (size_t textureIndex = 0; textureIndex < textures.usedSlots; textureIndex++)
	{
		const Texture *texture = ListGet(textures, textureIndex);
		VkImageView *textureImageView = malloc(sizeof(VkImageView *));
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
	const VkDescriptorPoolSize poolSizes[3] = {
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = MAX_FRAMES_IN_FLIGHT * MAX_FRAMES_IN_FLIGHT,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MAX_TEXTURES * MAX_FRAMES_IN_FLIGHT,
		},
		{
			.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = MAX_FRAMES_IN_FLIGHT,
		},
	};
	const VkDescriptorPoolCreateInfo poolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets = MAX_FRAMES_IN_FLIGHT,
		.poolSizeCount = 3,
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
		.pNext = NULL,
		.descriptorPool = descriptorPool,
		.descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
		.pSetLayouts = layouts,
	};

	VulkanTest(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets),
			   "Failed to allocate Vulkan descriptor sets!");

	VkDescriptorImageInfo imageInfo[textures.usedSlots];
	for (size_t textureIndex = 0; textureIndex < textures.usedSlots; textureIndex++)
	{
		imageInfo[textureIndex] = (VkDescriptorImageInfo){
			.sampler = textureSamplers.nearestRepeat,
			.imageView = ListGet(texturesImageView, textureIndex),
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
	}
	for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (textures.usedSlots == 0)
		{
			break;
		}
		const VkWriteDescriptorSet writeDescriptor = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = descriptorSets[i],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = textures.usedSlots,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = imageInfo,
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL,
		};
		vkUpdateDescriptorSets(device, 1, &writeDescriptor, 0, NULL);
	}

	UpdateUniformBufferDescriptorSets();

	return true;
}

bool CreateCommandBuffers()
{
	const VkCommandBufferAllocateInfo allocateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = graphicsCommandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MAX_FRAMES_IN_FLIGHT,
	};

	VulkanTest(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers),
			   "Failed to allocate Vulkan command buffers!");

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
