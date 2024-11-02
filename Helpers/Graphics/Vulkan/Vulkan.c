//
// Created by Noah on 7/5/2024.
//

#include <cglm/cglm.h>
#include <cglm/clipspace/persp_lh_no.h>
#include <cglm/clipspace/view_lh.h>
#include "Vulkan.h"
#include "../../Core/Timing.h"
#include "../../../Assets/AssetReader.h"
#include "../../../Assets/Assets.h"

#define List(type) struct {uint64_t length;type* data;}

typedef struct
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t transferFamily;
} QueueFamilyIndices;

typedef struct
{
    uint32_t formatCount;
    VkSurfaceFormatKHR *formats;
    uint32_t presentModeCount;
    VkPresentModeKHR *presentMode;
    VkSurfaceCapabilitiesKHR capabilities;
} SwapChainSupportDetails;

typedef struct
{
    vec2 pos;
    vec3 color;
} Vertex;

typedef struct
{
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct
{
    VkSurfaceFormatKHR chosenFormat;
    bool found;
} SwapSurfaceFormatCheck;

const List(Vertex) vertices = {
    4,
    (Vertex[]){
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    }
};

const List(uint16_t) indices = {6, (uint16_t[]){0, 1, 2, 2, 3, 0}};

static void VulkanError(const char *error)
{
    fflush(stdout);
    printf("%s", error);
    fflush(stderr);
}

/// A Vulkan instance is the connection between the game and the driver, through Vulkan.
/// The creation of it requires configuring Vulkan for the app, allowing for better driver performance.
VkInstance instance;
/// The interface between Vulkan and SDL, allowing Vulkan to actually draw to the window.
VkSurfaceKHR surface;
/// This stores the GPU.
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
QueueFamilyIndices queueFamilyIndices = {-1, -1, -1};
SwapChainSupportDetails swapChainSupport;
/// This is used for interfacing with the physical device.
VkDevice device;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue graphicsQueue;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue presentQueue;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue transferQueue;
/// Allows Vulkan to give a surface the rendered image.
VkSwapchainKHR swapChain;
VkImage *swapChainImages;
uint32_t swapChainCount;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkRenderPass renderPass;
VkDescriptorSetLayout descriptorSetLayout;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkFramebuffer *swapChainFramebuffers;
VkCommandPool graphicsCommandPool;
VkCommandPool transferCommandPool;
VkCommandBuffer *commandBuffers;
VkSemaphore *imageAvailableSemaphores;
VkSemaphore *renderFinishedSemaphores;
VkFence *inFlightFences;
uint8_t currentFrame = 0;
VkBuffer vertexBuffer;
VkDeviceMemory vertexBufferMemory;
VkBuffer indexBuffer;
VkDeviceMemory indexBufferMemory;
VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];
VkDescriptorPool descriptorPool;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

/**
 * This function will create the Vulkan instance, set up for SDL.
 * @param window The window to initialize Vulkan for.
 * @see instance
 */
static bool CreateInstance(SDL_Window *window)
{
    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
    const char *extensionNames[extensionCount];
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames);
    VkApplicationInfo applicationInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        NULL,
        GAME_TITLE,
        VULKAN_VERSION,
        GAME_TITLE,
        VULKAN_VERSION,
        VK_API_VERSION_1_3
    };
    VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        NULL,
        0,
        &applicationInfo,
        0,
        NULL,
        extensionCount,
        extensionNames
    };
#ifdef VALIDATION_ENABLE
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan instance!");
        return false;
    }
    return true;
}

/**
 * Creates the Vulkan surface
 * @param window The window the surface should be linked to
 * @see surface
 */
static bool CreateSurface(SDL_Window *window)
{
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE)
    {
        VulkanError("Failed to create Vulkan window surface");
        return false;
    }
    return true;
}

/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
static SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice pDevice)
{
    SwapChainSupportDetails details = {0, NULL, 0, NULL};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, NULL);
    if (details.formatCount != 0)
    {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR *) * details.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, details.formats);
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0)
    {
        details.presentMode = malloc(sizeof(VkPresentModeKHR *) * details.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, details.presentMode);
    }
    return details;
}

/**
 * This function selects the GPU that will be used to render the game.
 * Assuming I did it right, it will pick the best GPU available.
 */
static bool PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0)
    {
        VulkanError("Failed to find any GPUs with Vulkan support!");
        return false;
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    bool match = false;
    for (uint32_t i = 0; i < deviceCount; i++)
    {
        const VkPhysicalDevice pDevice = devices[i];
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
        if (!deviceFeatures.geometryShader) continue;
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, NULL);
        VkQueueFamilyProperties families[familyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, families);
        for (uint32_t index = 0; index < familyCount; index++)
        {
            if (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                queueFamilyIndices.graphicsFamily = index;
            } else if (families[index].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                queueFamilyIndices.transferFamily = index;
            }
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, index, surface, &presentSupport);
            if (presentSupport)
            {
                queueFamilyIndices.presentFamily = index;
            }
            if (queueFamilyIndices.graphicsFamily != -1 && queueFamilyIndices.presentFamily != -1 && queueFamilyIndices.
                transferFamily != -1)
            {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL);
                if (extensionCount == 0) continue;
                VkExtensionProperties availableExtensions[extensionCount];
                vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, availableExtensions);
                for (uint32_t j = 0; j < extensionCount; j++)
                {
                    if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
                    {
                        swapChainSupport = QuerySwapChainSupport(pDevice);
                        if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0) continue;
                        VkPhysicalDeviceProperties deviceProperties;
                        vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
                        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                        {
                            physicalDevice = devices[i];
                            return true;
                        }
                        physicalDevice = devices[i];
                        match = true;
                        break;
                    }
                }
                break;
            }
        }
    }
    if (!match) { VulkanError("Could not find a suitable GPU!"); }
    return match;
}

/**
 * Creates the logical device that is used to interface with the physical device.
 */
static bool CreateLogicalDevice()
{
    const float queuePriority = 1;
    uint32_t queueCount = 0;
    VkDeviceQueueCreateInfo queueCreateInfo[3];
    queueCreateInfo[queueCount++] = (VkDeviceQueueCreateInfo){
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        NULL,
        0,
        queueFamilyIndices.graphicsFamily,
        1,
        &queuePriority
    };
    queueCreateInfo[queueCount++] = (VkDeviceQueueCreateInfo){
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        NULL,
        0,
        queueFamilyIndices.transferFamily,
        1,
        &queuePriority
    };
    if (queueFamilyIndices.presentFamily != queueFamilyIndices.graphicsFamily && queueFamilyIndices.presentFamily !=
        queueFamilyIndices.transferFamily)
    {
        queueCreateInfo[queueCount++] = (VkDeviceQueueCreateInfo){
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,
            0,
            queueFamilyIndices.presentFamily,
            1,
            &queuePriority
        };
    }
    VkPhysicalDeviceFeatures deviceFeatures = {VK_FALSE};
    deviceFeatures.logicOp = true;
    VkDeviceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        NULL,
        0,
        queueCount,
        queueCreateInfo,
        0,
        NULL,
        1,
        (const char *const[1]){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
        &deviceFeatures
    };
#ifdef VALIDATION_ENABLE
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]){"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan device!");
        return false;
    }
    vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.transferFamily, 0, &transferQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);
    return true;
}

/**
 * A helper function used in @c CreateSwapChain used to find the color format for the surface to use.
 * If found, the function will pick R8G8B8A8_SRGB, otherwise it will simply use the first format found.
 * @return A @c VkSurfaceFormatKHR struct that contains the format.
 */
static SwapSurfaceFormatCheck GetSwapSurfaceFormat()
{
    int fallback = -1;
    for (uint32_t i = 0; i < swapChainSupport.formatCount; i++)
    {
        const VkSurfaceFormatKHR format = swapChainSupport.formats[i];
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB || fallback == -1)
            {
                fallback = (int)i;
            } else if (format.format == VK_FORMAT_R8G8B8A8_SRGB)
            {
                return (SwapSurfaceFormatCheck){format, true};
            }
        }
    }
    if (fallback != -1)
    {
        return (SwapSurfaceFormatCheck){swapChainSupport.formats[fallback], true};
    }
    VulkanError("Unable to find suitable Vulkan swap chain color format!");
    return (SwapSurfaceFormatCheck){{}, false};
}

/**
 * A helper function used in @c CreateSwapChain used to find the present mode for the surface to use.
 * @return The best present mode available
 */
static VkPresentModeKHR GetSwapPresentMode()
{
    for (uint32_t i = 0; i < swapChainSupport.presentModeCount; i++)
    {
        const VkPresentModeKHR presentMode = swapChainSupport.presentMode[i];
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static bool CreateSwapChain(SDL_Window *window)
{
    const SwapSurfaceFormatCheck surfaceFormat = GetSwapSurfaceFormat();
    if (!surfaceFormat.found) return false;
    const VkPresentModeKHR presentMode = GetSwapPresentMode();
    VkExtent2D extent = swapChainSupport.capabilities.currentExtent;
    if (extent.width == UINT32_MAX || extent.height == UINT32_MAX)
    {
        int width, height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);
        extent.width = clamp(width, swapChainSupport.capabilities.minImageExtent.width,
                             swapChainSupport.capabilities.maxImageExtent.width);
        extent.height = clamp(height, swapChainSupport.capabilities.minImageExtent.height,
                              swapChainSupport.capabilities.maxImageExtent.height);
    }
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        NULL,
        0,
        surface,
        imageCount,
        surfaceFormat.chosenFormat.format,
        surfaceFormat.chosenFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_CONCURRENT,
        2,
        (const uint32_t[]){
            queueFamilyIndices.graphicsFamily,
            queueFamilyIndices.transferFamily,
            queueFamilyIndices.presentFamily
        },
        swapChainSupport.capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };
    if (queueFamilyIndices.presentFamily != queueFamilyIndices.graphicsFamily && queueFamilyIndices.presentFamily !=
        queueFamilyIndices.transferFamily)
    {
        createInfo.queueFamilyIndexCount++;
    }
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan swap chain!");
        return false;
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    swapChainImages = malloc(sizeof(VkImage *) * imageCount);
    swapChainCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);
    swapChainImageFormat = surfaceFormat.chosenFormat.format;
    swapChainExtent = extent;
    return true;
}

static bool CreateImageViews()
{
    swapChainImageViews = malloc(sizeof(VkImageView *) * swapChainCount);
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        VkImageViewCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            NULL,
            0,
            swapChainImages[i],
            VK_IMAGE_VIEW_TYPE_2D,
            swapChainImageFormat,
            {VK_COMPONENT_SWIZZLE_IDENTITY},
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };
        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS)
        {
            VulkanError("Failed to create Vulkan image views!");
            return false;
        }
    }
    return true;
}

static bool CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {
        0,
        swapChainImageFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference colorAttachmentRef = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        NULL,
        1,
        &colorAttachmentRef,
        NULL,
        NULL,
        0,
        NULL
    };
    VkSubpassDependency dependency = {
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0
    };
    const VkRenderPassCreateInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &dependency
    };
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan render pass!");
        return false;
    }
    return true;
}

static bool CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBinding = {
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        NULL
    };
    const VkDescriptorSetLayoutCreateInfo layoutInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &layoutBinding
    };
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptorSetLayout) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan descriptor set layout!");
        return false;
    }
    return true;
}

static VkShaderModule CreateShaderModule(const uint32_t *code, const size_t size)
{
    const VkShaderModuleCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        NULL,
        0,
        size - 16,
        code
    };
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)
    {
        VulkanError("Failed to create shader module!");
        return NULL;
    }
    return shaderModule;
}

static bool CreateGraphicsPipeline()
{
    uint8_t *vertShaderCode = DecompressAsset(gzvert_shader_basic);
    uint8_t *fragShaderCode = DecompressAsset(gzfrag_shader_basic);
    VkShaderModule vertShaderModule = CreateShaderModule((uint32_t *) vertShaderCode,
                                                         AssetGetSize(gzvert_shader_basic));
    VkShaderModule fragShaderModule = CreateShaderModule((uint32_t *) fragShaderCode,
                                                         AssetGetSize(gzfrag_shader_basic));
    if (!vertShaderModule || !fragShaderModule) return false;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        NULL,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertShaderModule,
        "main",
        NULL
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        NULL,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragShaderModule,
        "main",
        NULL
    };
    VkPipelineShaderStageCreateInfo shaderStages[2] = {vertShaderStageInfo, fragShaderStageInfo};
    const VkVertexInputBindingDescription bindingDescription = {
        0,
        sizeof(Vertex),
        VK_VERTEX_INPUT_RATE_VERTEX
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        &bindingDescription,
        2,
        (VkVertexInputAttributeDescription[2]){
            {
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, pos)
            },
            {
                1,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                offsetof(Vertex, color)
            }
        }
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        NULL,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        NULL,
        0,
        1,
        NULL,
        1,
        NULL
    };
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        NULL,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_CLOCKWISE,
        VK_FALSE,
        0,
        0,
        0,
        1
    };
    VkPipelineMultisampleStateCreateInfo multisampling = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        NULL,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        1,
        NULL,
        VK_FALSE,
        VK_FALSE
    };
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        VK_TRUE,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlending = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        NULL,
        0,
        VK_TRUE,
        VK_LOGIC_OP_COPY,
        1,
        &colorBlendAttachment,
        {0, 0, 0, 0}
    };
    List(VkDynamicState) dynamicStates = {2, (VkDynamicState[]){VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}};
    VkPipelineDynamicStateCreateInfo dynamicState = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        NULL,
        0,
        dynamicStates.length,
        dynamicStates.data
    };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        NULL,
        0,
        1,
        &descriptorSetLayout,
        0,
        NULL
    };
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS)
    {
        VulkanError("Failed to create pipeline layout!");
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        NULL,
        0,
        2,
        shaderStages,
        &vertexInputInfo,
        &inputAssembly,
        NULL,
        &viewportState,
        &rasterizer,
        &multisampling,
        NULL,
        &colorBlending,
        &dynamicState,
        pipelineLayout,
        renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS)
    {
        VulkanError("Failed to create graphics pipeline!");
        return false;
    }
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
    return true;
}

static bool CreateFramebuffers()
{
    swapChainFramebuffers = malloc(sizeof(VkFramebuffer *) * swapChainCount);
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo = {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            NULL,
            0,
            renderPass,
            1,
            attachments,
            swapChainExtent.width,
            swapChainExtent.height,
            1
        };
        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            VulkanError("Failed to create Vulkan framebuffer!");
            return false;
        }
    }
    return true;
}

static bool CreateCommandPools()
{
    const VkCommandPoolCreateInfo graphicsPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndices.graphicsFamily
    };
    if (vkCreateCommandPool(device, &graphicsPoolInfo, NULL, &graphicsCommandPool) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan graphics command pool!");
        return false;
    }
    const VkCommandPoolCreateInfo transferPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        NULL,
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        queueFamilyIndices.transferFamily
    };
    if (vkCreateCommandPool(device, &transferPoolInfo, NULL, &transferCommandPool) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan transfer command pool!");
        return false;
    }
    return true;
}

static bool CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usageFlags,
                         const VkMemoryPropertyFlags propertyFlags,
                         VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        NULL,
        0,
        size,
        usageFlags,
        VK_SHARING_MODE_CONCURRENT,
        2,
        (const uint32_t[]){
            queueFamilyIndices.graphicsFamily,
            queueFamilyIndices.transferFamily,
            queueFamilyIndices.presentFamily
        }
    };
    if (queueFamilyIndices.presentFamily != queueFamilyIndices.graphicsFamily && queueFamilyIndices.presentFamily !=
        queueFamilyIndices.transferFamily)
    {
        bufferInfo.queueFamilyIndexCount++;
    }
    if (vkCreateBuffer(device, &bufferInfo, NULL, buffer) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan buffer!");
        return false;
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if (memRequirements.memoryTypeBits & 1 << i &&
            (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
        {
            const VkMemoryAllocateInfo allocInfo = {
                VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                NULL,
                memRequirements.size,
                i
            };
            if (vkAllocateMemory(device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS)
            {
                VulkanError("Failed to allocate Vulkan buffer memory!");
                return false;
            }
            vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
            return true;
        }
    }
    VulkanError("Failed to find suitable memory type for Vulkan!");
    return false;
}

static void CopyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size)
{
    const VkCommandBufferAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        transferCommandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);
    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        NULL
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    const VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);
    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        0,
        NULL,
        0,
        1,
        &commandBuffer,
        0,
        NULL
    };
    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);
    vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);
}

static bool CreateVertexBuffer()
{
    const VkDeviceSize bufferSize = sizeof(vertices.data[0]) * vertices.length;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                      &stagingBufferMemory))
        return false;
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory))
        return false;
    CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    return true;
}

static bool CreateIndexBuffer()
{
    const VkDeviceSize bufferSize = sizeof(indices.data[0]) * indices.length;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
                      &stagingBufferMemory))
        return false;
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    if (!CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory))
        return false;
    CopyBuffer(stagingBuffer, indexBuffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
    return true;
}

static bool CreateUniformBuffers()
{
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        const VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i],
                     &uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
    return true;
}

static bool CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        MAX_FRAMES_IN_FLIGHT
    };
    const VkDescriptorPoolCreateInfo poolCreateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0,
        MAX_FRAMES_IN_FLIGHT,
        1,
        &poolSize
    };
    if (vkCreateDescriptorPool(device, &poolCreateInfo, NULL, &descriptorPool) != VK_SUCCESS)
    {
        VulkanError("Failed to create Vulkan descriptor pool!");
        return false;
    }
    return true;
}

static bool CreateDescriptorSets()
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { layouts[i] = descriptorSetLayout; }
    const VkDescriptorSetAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        NULL,
        descriptorPool,
        MAX_FRAMES_IN_FLIGHT,
        layouts
    };
    if (vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets) != VK_SUCCESS)
    {
        VulkanError("Failed to allocate Vulkan descriptor sets!");
        return false;
    }
    for (uint8_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo = {
            uniformBuffers[i],
            0,
            sizeof(UniformBufferObject)
        };
        VkWriteDescriptorSet writeDescriptors = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            NULL,
            descriptorSets[i],
            0,
            0,
            1,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            NULL,
            &bufferInfo,
            NULL
        };
        vkUpdateDescriptorSets(device, 1, &writeDescriptors, 0, NULL);
    }
    return true;
}

static bool CreateCommandBuffers()
{
    commandBuffers = malloc(sizeof(VkCommandBuffer *) * MAX_FRAMES_IN_FLIGHT);
    const VkCommandBufferAllocateInfo allocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        NULL,
        graphicsCommandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        MAX_FRAMES_IN_FLIGHT
    };
    if (vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers) != VK_SUCCESS)
    {
        VulkanError("Failed to allocate Vulkan command buffers!");
        return false;
    }
    return true;
}

static bool CreateSyncObjects()
{
    imageAvailableSemaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
    inFlightFences = malloc(sizeof(VkFence *) * MAX_FRAMES_IN_FLIGHT);
    const VkSemaphoreCreateInfo semaphoreInfo = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        NULL,
        0
    };
    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        NULL,
        VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) != VK_SUCCESS)
        {
            VulkanError("Failed to create Vulkan semaphores!");
            return false;
        }
    }
    return true;
}

static void RecordCommandBuffer(const VkCommandBuffer buffer, const uint32_t imageIndex)
{
    const VkCommandBufferBeginInfo beginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        NULL,
        0,
        NULL
    };
    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
    {
        VulkanError("Failed to begin recording Vulkan command buffer!");
    }
    VkClearValue clearColor = {{{0, 0, 0}}};
    const VkRenderPassBeginInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        NULL,
        renderPass,
        swapChainFramebuffers[imageIndex],
        {
            {0, 0},
            swapChainExtent
        },
        1,
        &clearColor
    };
    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    const VkViewport viewport = {
        0,
        0,
        (float) swapChainExtent.width,
        (float) swapChainExtent.height,
        0,
        1
    };
    vkCmdSetViewport(buffer, 0, 1, &viewport);
    const VkRect2D scissor = {
        {
            0,
            0
        },
        swapChainExtent
    };
    vkCmdSetScissor(buffer, 0, 1, &scissor);
    vkCmdBindVertexBuffers(buffer, 0, 1, (VkBuffer[1]){vertexBuffer}, (VkDeviceSize[1]){0});
    vkCmdBindIndexBuffer(buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &descriptorSets[currentFrame], 0, NULL);
    vkCmdDrawIndexed(buffer, indices.length, 1, 0, 0, 0);
    vkCmdEndRenderPass(buffer);
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
    {
        VulkanError("Failed to record the Vulkan command buffer!");
    }
}

/**
 * This function is used to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
bool InitVulkan(SDL_Window *window)
{
    // ReSharper disable once CppDFAConstantConditions
    if (!CreateInstance(window) || !CreateSurface(window) || !PickPhysicalDevice() || !CreateLogicalDevice() || !
        CreateSwapChain(window) || !CreateImageViews() || !CreateRenderPass() || !CreateDescriptorSetLayout() || !
        CreateGraphicsPipeline() || !CreateFramebuffers() || !CreateCommandPools() || !CreateVertexBuffer() || !
        CreateIndexBuffer() || !CreateUniformBuffers() || !CreateDescriptorPool() || !CreateDescriptorSets() || !
        CreateCommandBuffers() || !CreateSyncObjects())
    {
        CleanupVulkan();
        return false;
    }
    return true;
}

static void UpdateUniformBuffer(const uint32_t currentFrame)
{
    UniformBufferObject bufferObject = {
        GLM_MAT4_IDENTITY_INIT,
        GLM_MAT4_IDENTITY_INIT,
        GLM_MAT4_IDENTITY_INIT
    };
    glm_rotate(bufferObject.model, (float)SDL_GetTicks64() * PIf / 10000.0f, GLM_YUP);
    glm_lookat_lh((vec3){2.0f, 2.0f, 2.0f}, GLM_VEC3_ZERO, ((vec3){0.0f, -1.0f, 0.0f}), bufferObject.view);
    glm_perspective_lh_no(PI / 4, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f, bufferObject.proj);
    memcpy(uniformBuffersMapped[currentFrame], &bufferObject, sizeof(bufferObject));
}

void DrawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE,
                          &imageIndex);
    UpdateUniformBuffer(currentFrame);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    const VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        NULL,
        1,
        waitSemaphores,
        waitStages,
        1,
        &commandBuffers[currentFrame],
        1,
        signalSemaphores
    };
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        VulkanError("Failed to submit Vulkan draw command buffer!");
    }
    const VkSwapchainKHR swapChains[] = {swapChain};
    const VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        NULL,
        1,
        signalSemaphores,
        1,
        swapChains,
        &imageIndex,
        NULL
    };
    vkQueuePresentKHR(presentQueue, &presentInfo);
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/// A function used to destroy the Vulkan objects when they are no longer needed.
void CleanupVulkan()
{
    vkDeviceWaitIdle(device);
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
    }
    for (uint32_t i = 0; i < swapChainCount; i++)
    {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(device, uniformBuffers[i], NULL);
        vkFreeMemory(device, uniformBuffersMemory[i], NULL);
    }
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyBuffer(device, indexBuffer, NULL);
    vkFreeMemory(device, indexBufferMemory, NULL);
    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }
    vkDestroyCommandPool(device, graphicsCommandPool, NULL);
    vkDestroyCommandPool(device, transferCommandPool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
}
