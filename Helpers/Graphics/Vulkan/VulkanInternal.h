//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective
#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE

#include <SDL_vulkan.h>
#include <string.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include "Vulkan.h"
#include "../../../Assets/AssetReader.h"
#include "../../../Assets/Assets.h"
#include "../../Core/DataReader.h"
#include "../../Core/Error.h"
#include "../../Core/MathEx.h"

#pragma region macros
#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#define MAX_FRAMES_IN_FLIGHT 2
#define VulkanLogError(error) printf("\033[31m%s\033[0m\n", error); fflush(stdout)
#define VulkanTest_Internal(function, error, returnValue) {const VkResult result=function; if (result != VK_SUCCESS) { printf("\033[31m%s\033[0m Error code %d\n", error, result); fflush(stdout); return returnValue; }}
#define VulkanTest(function, error) VulkanTest_Internal(function, error, false)
#define List(type) struct {uint64_t length;type* data;}
#pragma endregion macros

#pragma region typedefs
typedef struct
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t uniquePresentFamily;
    uint32_t transferFamily;
    uint8_t familyCount;
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
    vec3 pos;
    vec3 color;
    vec2 textureCoordinate;
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

typedef struct
{
    VkImage image;
    VkMemoryRequirements memoryRequirements;
    VkDeviceSize offset;
} ImageAllocationInformation;

typedef struct
{
    uint16_t textureIndex;
} DataBufferObject;
#pragma endregion typedefs

#pragma region variables
SDL_Window *vk_window;
bool minimized = false;

const List(Vertex) vertices = {
    8,
    (Vertex[]){
            {{-0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    }
};

const List(uint16_t) indices = {
    12, (uint16_t[]){
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    }
};
#pragma endregion variables

#pragma region vulkanVariables
/// When the instance is created the Vulkan library gets initialized, allowing the game to provide the library with any
/// information about itself. Any state information that the library provides will then be stored in the instance.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html
VkInstance instance = VK_NULL_HANDLE;
/// The interface between Vulkan and SDL, allowing Vulkan to actually interact with the window.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceKHR.html
VkSurfaceKHR surface;
/// The physical device is the hardware available to the host that has an implementation of Vulkan.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-physical-device-enumeration
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html
VkPhysicalDevice physicalDevice;
/// @todo Document this along with the struct
QueueFamilyIndices *queueFamilyIndices;
/// @todo Document this along with the struct
SwapChainSupportDetails swapChainSupport;
/// The logical device is a connection to a physical device, and is used for interfacing with Vulkan.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-devices
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html
VkDevice device = NULL;
/// The graphics queue is the queue used for executing graphics command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
VkQueue graphicsQueue;
/// The present queue is the queue used for executing present command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
VkQueue presentQueue;
/// The transfer queue is the queue used for executing transfer command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
VkQueue transferQueue;
/// Allows Vulkan to give a surface the rendered image.
VkSwapchainKHR swapChain = VK_NULL_HANDLE;
VkImage *swapChainImages;
uint32_t swapChainCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView *swapChainImageViews;
VkRenderPass renderPass = VK_NULL_HANDLE;
VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
VkPipeline graphicsPipeline = VK_NULL_HANDLE;
VkFramebuffer *swapChainFramebuffers;
VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandPool = VK_NULL_HANDLE;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
bool framebufferResized = false;
uint8_t currentFrame = 0;
VkBuffer vertexBuffer = VK_NULL_HANDLE;
VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
VkBuffer indexBuffer = VK_NULL_HANDLE;
VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];
VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
ImageAllocationInformation textures[TEXTURE_ASSET_COUNT];
VkDeviceMemory textureMemory;
VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
uint16_t texturesAssetIDMap[ASSET_COUNT];
VkSampler textureSampler;
VkBuffer dataBuffer = VK_NULL_HANDLE;
VkDeviceMemory dataBufferMemory = VK_NULL_HANDLE;
void *mappedDataBuffer;
VkFormat depthImageFormat;
VkImage depthImage;
VkDeviceMemory depthImageMemory;
VkImageView depthImageView;
#pragma endregion vulkanVariables

#pragma region internalFunctions
#pragma region helperFunctions
/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
static void QuerySwapChainSupport(VkPhysicalDevice pDevice);

/**
 * A helper function used in @c CreateSwapChain used to find the color format for the surface to use.
 * If found, the function will pick R8G8B8A8_SRGB, otherwise it will simply use the first format found.
 * @return A @c VkSurfaceFormatKHR struct that contains the format.
 */
static SwapSurfaceFormatCheck GetSwapSurfaceFormat();

/**
 * A helper function used in @c CreateSwapChain used to find the present mode for the surface to use.
 * @return The best present mode available
 */
static VkPresentModeKHR GetSwapPresentMode();


static VkShaderModule CreateShaderModule(const uint32_t *code, const size_t size);

static bool CreateImage(VkImage *image, VkFormat format, VkExtent3D extent, VkImageUsageFlags usageFlags, const char *errorMessage);

static bool CreateImageView(VkImageView *imageView, VkImage image, VkFormat format, VkImageAspectFlagBits aspectMask, const char *errorMessage);

static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory);

static VkCommandBuffer BeginCommandBuffer();

static void EndCommandBuffer(VkCommandBuffer commandBuffer);

static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

static void CleanupSwapChain();

#pragma region drawingFunctions
static bool RecreateSwapChain();

static void UpdateUniformBuffer(uint32_t currentFrame);

static void RecordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
#pragma endregion drawingFunctions
#pragma endregion helperFunctions
/**
 * This function will create the Vulkan instance, set up for SDL.
 * @see instance
 */
static bool CreateInstance();

/**
 * Creates the Vulkan surface
 * @see surface
 */
static bool CreateSurface();

/**
 * This function selects the GPU that will be used to render the game.
 * Assuming I did it right, it will pick the best GPU available.
 */
static bool PickPhysicalDevice();

/**
 * Creates the logical device that is used to interface with the physical device.
 */
static bool CreateLogicalDevice();

static bool CreateSwapChain();

static bool CreateImageViews();

static bool CreateRenderPass();

static bool CreateDescriptorSetLayout();

static bool CreateGraphicsPipeline();

static bool CreateCommandPools();

static bool CreateDepthImage();

static bool CreateFramebuffers();

static bool LoadTextures();

static bool CreateTexturesImageView();

static bool CreateTextureSampler();

static bool CreateVertexBuffer();

static bool CreateIndexBuffer();

static bool CreateUniformBuffers();

static bool CreateDescriptorPool();

static bool CreateDescriptorSets();

static bool CreateCommandBuffers();

static bool CreateSyncObjects();
#pragma endregion internalFunctions

#endif //VULKANINTERNAL_H
