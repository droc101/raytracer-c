//
// Created by Noah on 7/5/2024.
//

#include "Vulkan.h"
#include "Error.h"
#include "../Assets/AssetReader.h"
#include "../Assets/Assets.h"

/// A Vulkan instance is the connection between the game and the driver, through Vulkan.
/// The creation of it requires configuring Vulkan for the app, allowing for better driver performance.
VkInstance instance;
/// The interface between Vulkan and SDL, allowing Vulkan to actually draw to the window.
VkSurfaceKHR surface;
/// This stores the GPU.
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
QueueFamilyIndices queueFamilyIndices;
SwapChainSupportDetails swapChainSupport;
/// This is used for interfacing with the physical device.
VkDevice device;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue graphicsQueue;
/// Async GPU (I thought I escaped async/await 😭)
VkQueue presentQueue;
/// Allows Vulkan to give a surface the rendered image.
VkSwapchainKHR swapChain;
VkImage *swapChainImages;
unsigned int swapChainCount;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkFramebuffer *swapChainFramebuffers;
VkCommandPool commandPool;
VkCommandBuffer *commandBuffers;
VkSemaphore *imageAvailableSemaphores;
VkSemaphore *renderFinishedSemaphores;
VkFence *inFlightFences;
unsigned int currentFrame = 0;

/**
 * This function finds the indices of the queue families in QueueFamilyIndices.
 * @param pDevice The device to search within
 * @return A @c QueueFamilyIndices struct with values set either to the index of the queue family, or to `(unsigned int) -1` if the queue was not found
 * @see https://docs.vulkan.org/guide/latest/queues.html#_queue_family
 */
static inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice pDevice) {
    QueueFamilyIndices indices = {-1, -1};
    unsigned int familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, NULL);
    VkQueueFamilyProperties families[familyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familyCount, families);
    for (unsigned int i = 0; i < familyCount; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.graphicsFamily != 0 && indices.presentFamily != 0) break;
    }
    return indices;
}

/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
static inline SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice pDevice) {
    SwapChainSupportDetails details = {0, NULL, 0, NULL};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, NULL);
    if (details.formatCount != 0) {
        details.formats = malloc(sizeof(VkSurfaceFormatKHR *) * details.formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &details.formatCount, details.formats);
    }
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, NULL);
    if (details.presentModeCount != 0) {
        details.presentMode = malloc(sizeof(VkPresentModeKHR *) * details.presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &details.presentModeCount, details.presentMode);
    }
    return details;
}

/**
 * This function will create the Vulkan instance, set up for SDL.
 * @param window The window to initialize Vulkan for.
 * @see instance
 */
static inline void CreateInstance(SDL_Window *window) {
    unsigned int extensionCount = 0;
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
    createInfo.ppEnabledLayerNames = (const char *const[1]) {"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
        Error("Failed to create Vulkan instance!");
    }
}

/**
 * Creates the Vulkan surface
 * @param window The window the surface should be linked to
 * @see surface
 */
static inline void CreateSurface(SDL_Window *window) {
    if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE) {
        Error("Failed to create Vulkan window surface");
    }
}

/**
 * This function selects the GPU that will be used to render the game.
 * Assuming I did it right, it will pick the best GPU available.
 * @see FindQueueFamilies
 */
static inline void PickPhysicalDevice() {
    unsigned int deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        Error("Failed to find any GPUs with Vulkan support!");
    }
    VkPhysicalDevice devices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    bool match = false;
    for (unsigned int i = 0; i < deviceCount; i++) {
        VkPhysicalDevice pDevice = devices[i];
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(pDevice, &deviceFeatures);
        if (!deviceFeatures.geometryShader) continue;
        queueFamilyIndices = FindQueueFamilies(pDevice);
        if (queueFamilyIndices.graphicsFamily == -1) continue;
        if (queueFamilyIndices.presentFamily == -1) continue;
        unsigned int extensionCount;
        vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL);
        if (extensionCount == 0) continue;
        VkExtensionProperties availableExtensions[extensionCount];
        vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, availableExtensions);
        bool extensionFound = false;
        for (unsigned int j = 0; j < extensionCount; j++) {
            if (strcmp(availableExtensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) continue;
        swapChainSupport = QuerySwapChainSupport(pDevice);
        if (swapChainSupport.formatCount == 0 && swapChainSupport.presentModeCount == 0) continue;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(pDevice, &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physicalDevice = devices[i];
            return;
        } else {
            physicalDevice = devices[i];
            match = true;
        }
    }
    if (!match) Error("Could not find a suitable GPU!");
}

/**
 * Creates the logical device that is used to interface with the physical device.
 * @see FindQueueFamilies
 */
static inline void CreateLogicalDevice() {
    float queuePriority = 1;
    unsigned int queueCount = 1;
    VkDeviceQueueCreateInfo queueCreateInfo[2];
    queueCreateInfo[0] = (VkDeviceQueueCreateInfo) {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,
            0,
            queueFamilyIndices.graphicsFamily,
            1,
            &queuePriority
    };
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        queueCreateInfo[queueCount++] = (VkDeviceQueueCreateInfo) {
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
            (const char *const[1]) {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
            &deviceFeatures
    };
#ifdef VALIDATION_ENABLE
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = (const char *const[1]) {"VK_LAYER_KHRONOS_validation"};
#endif
    if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
        Error("Failed to create Vulkan device!");
    }
    vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);
}

/**
 * A helper function used in @c CreateSwapChain used to find the color format for the surface to use.
 * If found, the function will pick R8G8B8A8_SRGB, otherwise it will simply use the first format found.
 * @return A @c VkSurfaceFormatKHR struct that contains the format.
 */
static inline VkSurfaceFormatKHR GetSwapSurfaceFormat() {
    for (unsigned int i = 0; i < swapChainSupport.formatCount; i++) {
        VkSurfaceFormatKHR format = swapChainSupport.formats[i];
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return swapChainSupport.formats[0];
}

/**
 * A helper function used in @c CreateSwapChain used to find the present mode for the surface to use.
 * @return The best present mode available
 */
static inline VkPresentModeKHR GetSwapPresentMode() {
    for (unsigned int i = 0; i < swapChainSupport.presentModeCount; i++) {
        VkPresentModeKHR presentMode = swapChainSupport.presentMode[i];
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static inline void CreateSwapChain() {
    VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat();
    VkPresentModeKHR presentMode = GetSwapPresentMode();
    unsigned int imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            NULL,
            0,
            surface,
            imageCount,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            swapChainSupport.capabilities.currentExtent,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            0,
            NULL,
            swapChainSupport.capabilities.currentTransform,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            presentMode,
            VK_TRUE,
            VK_NULL_HANDLE
    };
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = (const unsigned int[2]) {queueFamilyIndices.graphicsFamily,
                                                                  queueFamilyIndices.presentFamily};
    }
    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) {
        Error("Failed to create Vulkan swap chain!");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);
    swapChainImages = malloc(sizeof(VkImage *) * imageCount);
    swapChainCount = imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = swapChainSupport.capabilities.currentExtent;
}

static inline void CreateImageViews() {
    swapChainImageViews = malloc(sizeof(VkImageView *) * swapChainCount);
    for (unsigned int i = 0; i < swapChainCount; i++) {
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
        if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
            Error("Failed to create Vulkan image views!");
        }
    }
}

static inline VkShaderModule CreateShaderModule(const unsigned int *code, unsigned long long size) {
    VkShaderModuleCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            NULL,
            0,
            size - 16,
            code
    };
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        Error("Failed to create shader module!");
    }
    return shaderModule;
}

static inline void CreateRenderPass() {
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
    VkRenderPassCreateInfo renderPassInfo = {
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
    if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) {
        Error("Failed to create Vulkan render pass!");
    }
}

static inline void CreateGraphicsPipeline() {
    unsigned char *vertShaderCode = DecompressAsset(gzvert_shader_basic);
    unsigned char *fragShaderCode = DecompressAsset(gzfrag_shader_basic);
    VkShaderModule vertShaderModule = CreateShaderModule((unsigned int *) vertShaderCode,
                                                         AssetGetSize(gzvert_shader_basic));
    VkShaderModule fragShaderModule = CreateShaderModule((unsigned int *) fragShaderCode,
                                                         AssetGetSize(gzfrag_shader_basic));
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
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            NULL,
            0,
            0,
            NULL,
            0,
            NULL
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
            VK_CULL_MODE_BACK_BIT,
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
#define dynamicStateCount 2 // Yes, this is somewhat cursed, but if I used a const instead of a #define it makes dynamicStates become a dynamic length array
    VkDynamicState dynamicStates[dynamicStateCount] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            NULL,
            0,
            dynamicStateCount,
            dynamicStates
    };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            0,
            NULL,
            0,
            NULL
    };
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        Error("Failed to create pipeline layout!");
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
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
        Error("Failed to create graphics pipeline!");
    }
    vkDestroyShaderModule(device, vertShaderModule, NULL);
    vkDestroyShaderModule(device, fragShaderModule, NULL);
}

static inline void CreateFramebuffers() {
    swapChainFramebuffers = malloc(sizeof(VkFramebuffer *) * swapChainCount);
    for (unsigned int i = 0; i < swapChainCount; i++) {
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
        if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            Error("Failed to create Vulkan framebuffer!");
        }
    }
}

static inline void CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            NULL,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            queueFamilyIndices.graphicsFamily
    };
    if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
        Error("Failed to create Vulkan command pool!");
    }
}

static inline void CreateCommandBuffers() {
    commandBuffers = malloc(sizeof(VkCommandBuffer *) * MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            NULL,
            commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            MAX_FRAMES_IN_FLIGHT
    };
    if (vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers) != VK_SUCCESS) {
        Error("Failed to allocate Vulkan command buffers!");
    }
}

static inline void CreateSyncObjects() {
    imageAvailableSemaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores = malloc(sizeof(VkSemaphore *) * MAX_FRAMES_IN_FLIGHT);
    inFlightFences = malloc(sizeof(VkFence *) * MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            NULL,
            0
    };
    VkFenceCreateInfo fenceInfo = {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            NULL,
            VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) != VK_SUCCESS) {
            Error("Failed to create Vulkan semaphores!");
        }
    }

}

static inline void RecordCommandBuffer(VkCommandBuffer buffer, unsigned int imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            NULL,
            0,
            NULL
    };
    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        Error("Failed to begin recording Vulkan command buffer!");
    }
    VkClearValue clearColor = {{{0, 0, 0}}};
    VkRenderPassBeginInfo renderPassInfo = {
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
    VkViewport viewport = {
            0,
            0,
            (float) swapChainExtent.width,
            (float) swapChainExtent.height,
            0,
            1
    };
    vkCmdSetViewport(buffer, 0, 1, &viewport);
    VkRect2D scissor = {
            {
                    0,
                    0
            },
            swapChainExtent
    };
    vkCmdSetScissor(buffer, 0, 1, &scissor);
    vkCmdDraw(buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(buffer);
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        Error("Failed to record the Vulkan command buffer!");
    }
}

/**
 * This function is used to to create the Vulkan instance and surface, as well as configuring the environment properly.
 * This function (and the functions it calls) do NOT perform any drawing, though the framebuffers are initialized here.
 * @param window The window to initialize Vulkan for.
 * @see CreateInstance
 * @see PickPhysicalDevice
 * @see CreateLogicalDevice
 */
void InitVulkan(SDL_Window *window) {
    CreateInstance(window);
    CreateSurface(window);
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void DrawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    unsigned int imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkSubmitInfo submitInfo = {
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
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        Error("Failed to submit Vulkan draw command buffer!");
    }
    VkSwapchainKHR swapChains[] = {swapChain};
    VkPresentInfoKHR presentInfo = {
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
void CleanupVulkan() {
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
        vkDestroyFence(device, inFlightFences[i], NULL);
    }
    vkDestroyCommandPool(device, commandPool, NULL);
    for (unsigned int i = 0; i < swapChainCount; i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
    }
    vkDestroyPipeline(device, graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);
    for (unsigned int i = 0; i < swapChainCount; i++) {
        vkDestroyImageView(device, swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(device, swapChain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
}

/// @return The Vulkan instance
VkInstance GetVulkanInstance() {
    return instance;
}

/// @return The Vulkan surface
VkSurfaceKHR GetVulkanSurface() {
    return surface;
}

/// @return The Vulkan device
VkDevice GetVulkanDevice() {
    return device;
}