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
#include "Vulkan.h"
#include "../../../Assets/AssetReader.h"
#include "../../../Assets/Assets.h"
#include "../../Core/DataReader.h"
#include "../../Core/Error.h"
#include "../../Core/Logging.h"
#include "../../Core/MathEx.h"

#pragma region macros
#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#define MAX_FRAMES_IN_FLIGHT 2
#define VulkanLogError(message, ...) LogInternal("VULKAN", 31, true, message, __VA_OPT__(__VA_ARGS__, ) "")
// TODO Use LogInternal
#define VulkanTestInternal(function, returnValue, ...) { \
    const VkResult result=function; \
    if (result != VK_SUCCESS) { \
        printf("\033[31m"); \
        printf(__VA_ARGS__); \
        printf("\033[0m Error code %d\n", result); \
        if (result == VK_ERROR_DEVICE_LOST) { \
            printf("See https://starflight.dev/media/VK_ERROR_DEVICE_LOST.png for more information\n"); \
        } \
        fflush(stdout); \
        return returnValue; \
    } \
}
#define VulkanTestWithReturn(function, returnValue, ...) VulkanTestInternal(function, returnValue, __VA_ARGS__)
#define VulkanTestReturnResult(function, ...) VulkanTestInternal(function, result, __VA_ARGS__)
#define VulkanTest(function, ...) VulkanTestInternal(function, false, __VA_ARGS__)
#define List(type) struct {uint64_t length;type* data;}
#pragma endregion macros

#pragma region typedefs
/// A struct to hold the indicies of the queue families for graphics, presentation, and transfer.
/// This is used to find and store the indices, which allows for picking between unique and non-unique indices.
typedef struct QueueFamilyIndices
{
    /// The index of the family on the GPU that will be used for graphics processing
    uint32_t graphicsFamily;
    /** The index of the family on the GPU that will be used for presentation
     * @note If the graphics family supports presentation, @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::graphicsFamily
     * @note Similarly, if the graphics family does not support presentation then @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::uniquePresentFamily
     */
    uint32_t presentFamily;
    /// If the graphics family does not support presentation this will contain the same value as @c QueueFamilyIndices::presentFamily
    uint32_t uniquePresentFamily;
    /// The index of the family on the GPU that will be used for transfer operations
    uint32_t transferFamily;
    /** The total count of unique families
     * @note If this is 1, then @c QueueFamilyIndices::graphicsFamily, @c QueueFamilyIndices::presentFamily, and @c QueueFamilyIndices::transferFamily will all have the same value. If this is the case, then @c QueueFamilyIndices::uniquePresentFamily will be @c UINT32_MAX
     * @note If this is 2 and @code QueueFamilyIndices::presentFamily == QueueFamilyIndices::uniquePresentFamily @endcode, then @c QueueFamilyIndices::graphicsFamily and @c QueueFamilyIndices::transferFamily will have the same value.
     * @note If this is 2 and @code QueueFamilyIndices::presentFamily != QueueFamilyIndices::uniquePresentFamily @endcode, then @c QueueFamilyIndices::graphicsFamily and @c QueueFamilyIndices::presentFamily will have the same value.
     * @note If this is 3, then @c QueueFamilyIndices::graphicsFamily, @c QueueFamilyIndices::presentFamily, and @c QueueFamilyIndices::transferFamily will all have unique values. If this is the case, then @c QueueFamilyIndices::presentFamily will be equal to @c QueueFamilyIndices::uniquePresentFamily
     */
    uint8_t familyCount;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails
{
    uint32_t formatCount;
    VkSurfaceFormatKHR *formats;
    uint32_t presentModeCount;
    VkPresentModeKHR *presentMode;
    VkSurfaceCapabilitiesKHR capabilities;
} SwapChainSupportDetails;

typedef struct Vertex
{
    vec3 pos;
    vec3 color;
    vec2 textureCoordinate;
} Vertex;

typedef struct UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct ImageAllocationInformation
{
    VkImage image;
    VkExtent3D extent;
    uint8_t mipmapLevels;
    VkMemoryRequirements memoryRequirements;
    VkDeviceSize offset;
} ImageAllocationInformation;

typedef struct DataBufferObject
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
    12,
    (uint16_t[]){
        0,
        1,
        2,
        2,
        3,
        0,
        4,
        5,
        6,
        6,
        7,
        4
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
SwapChainSupportDetails *swapChainSupport;
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
VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;
#pragma endregion vulkanVariables

#pragma region internalFunctions
#pragma region helperFunctions
/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
static bool QuerySwapChainSupport(VkPhysicalDevice pDevice);

static bool CreateImageView(VkImageView *imageView, VkImage image, VkFormat format, VkImageAspectFlagBits aspectMask,
                            uint8_t mipmapLevels, const char *errorMessage);

static VkShaderModule CreateShaderModule(const uint32_t *code, size_t size);

static bool CreateImage(VkImage *image, VkDeviceMemory *imageMemory, VkFormat format, VkExtent3D extent,
                        uint8_t mipmapLevels, VkSampleCountFlags samples,
                        VkImageUsageFlags usageFlags, const char *imageType);

static bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, VkCommandPool commandPool);

static bool EndCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue);

static bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags,
                         VkMemoryPropertyFlags propertyFlags, VkBuffer *buffer, VkDeviceMemory *bufferMemory);

static bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

static void CleanupSwapChain();

static void CleanupColorImage();

static void CleanupDepthImage();

static bool RecreateSwapChain();
#pragma region drawingFunctions
static void UpdateUniformBuffer(uint32_t currentFrame);

static bool RecordCommandBuffer(VkCommandBuffer buffer, uint32_t imageIndex);
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

// TODO Use multiple queues from queue families, at least when sub-optimal family setups are in use
static bool CreateLogicalDevice();

static bool CreateSwapChain();

static bool CreateImageViews();

static bool CreateRenderPass();

static bool CreateDescriptorSetLayout();

static bool CreateGraphicsPipeline();

static bool CreateCommandPools();

static bool CreateColorImage();

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
