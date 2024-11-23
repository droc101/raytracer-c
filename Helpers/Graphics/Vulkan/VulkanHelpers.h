//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <SDL_video.h>
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include "../../../Assets/Assets.h"
#include "../../Core/Logging.h"

#pragma region macros
#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#define MAX_FRAMES_IN_FLIGHT 2
#define UI_PRIMITIVES 2048

#define VulkanLogError(...) LogInternal("VULKAN", 31, true, __VA_ARGS__)
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
#define GetColor(color) const float r = (color >> 16 & 0xFF) / 255.0f; const float g = (color >> 8 & 0xFF) / 255.0f; const float b = (color & 0xFF) / 255.0f; const float a = (color >> 24 & 0xFF) / 255.0f

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

typedef struct UiVertex
{
    vec4 posXY_uvZW;
    vec4 color;
    uint32_t textureIndex;
} UiVertex;

typedef struct WallVertex
{
    vec3 pos;
    vec2 uv;
} WallVertex;

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

typedef struct VertexBuffer
{
    VkBuffer buffer;
    UiVertex *vertices; // TODO Verify start size
    UiVertex *fallback;
    uint32_t vertexCount;
    uint32_t maxVertices;
    uint32_t fallbackMaxVertices;
} VertexBuffer;

typedef struct VertexBuffers
{
    VertexBuffer walls;
    VkDeviceMemory localMemory;

    VertexBuffer ui;
    VkDeviceMemory sharedMemory;
} VertexBuffers;

typedef struct Pipelines
{
    VkPipeline walls;
    VkPipeline ui;
} Pipelines;
#pragma endregion typedefs

#pragma region variables
extern SDL_Window *vk_window;
extern bool minimized;

/// When the instance is created the Vulkan library gets initialized, allowing the game to provide the library with any
/// information about itself. Any state information that the library provides will then be stored in the instance.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html
extern VkInstance instance;
/// The interface between Vulkan and SDL, allowing Vulkan to actually interact with the window.
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceKHR.html
extern VkSurfaceKHR surface;
/// The physical device is the hardware available to the host that has an implementation of Vulkan.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-physical-device-enumeration
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html
extern VkPhysicalDevice physicalDevice;
/// @todo Document this along with the struct
extern QueueFamilyIndices *queueFamilyIndices;
/// @todo Document this along with the struct
extern SwapChainSupportDetails *swapChainSupport;
/// The logical device is a connection to a physical device, and is used for interfacing with Vulkan.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-devices
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html
extern VkDevice device;
/// The graphics queue is the queue used for executing graphics command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue graphicsQueue;
/// The present queue is the queue used for executing present command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue presentQueue;
/// The transfer queue is the queue used for executing transfer command buffers and sparse bindings on the device.
/// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-queues
/// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueue.html
extern VkQueue transferQueue;
/// Allows Vulkan to give a surface the rendered image.
extern VkSwapchainKHR swapChain;
extern VkImage *swapChainImages;
extern uint32_t swapChainCount;
extern VkFormat swapChainImageFormat;
extern VkExtent2D swapChainExtent;
extern VkImageView *swapChainImageViews;
extern VkRenderPass renderPass;
extern VkDescriptorSetLayout descriptorSetLayout;
extern VkPipelineLayout pipelineLayout;
extern Pipelines pipelines;
extern VkFramebuffer *swapChainFramebuffers;
extern VkCommandPool graphicsCommandPool;
extern VkCommandPool transferCommandPool;
extern VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
extern bool framebufferResized;
extern uint8_t currentFrame;
extern uint32_t swapchainImageIndex;
extern VertexBuffers vertexBuffers;
extern VkBuffer indexBuffer;
extern VkDeviceMemory indexBufferMemory;
extern VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
extern void *uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];
extern VkDescriptorPool descriptorPool;
extern VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
extern ImageAllocationInformation textures[TEXTURE_ASSET_COUNT];
extern VkDeviceMemory textureMemory;
extern VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
extern uint16_t texturesAssetIDMap[ASSET_COUNT];
extern VkSampler textureSampler;
extern VkBuffer dataBuffer;
extern VkDeviceMemory dataBufferMemory;
extern void *mappedDataBuffer;
extern VkFormat depthImageFormat;
extern VkImage depthImage;
extern VkDeviceMemory depthImageMemory;
extern VkImageView depthImageView;
extern VkImage colorImage;
extern VkDeviceMemory colorImageMemory;
extern VkImageView colorImageView;
#pragma endregion variables

#pragma region helperFunctions
/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
bool QuerySwapChainSupport(VkPhysicalDevice pDevice);

bool CreateImageView(VkImageView *imageView, VkImage image, VkFormat format, VkImageAspectFlagBits aspectMask,
                            uint8_t mipmapLevels, const char *errorMessage);

VkShaderModule CreateShaderModule(const uint32_t *code, size_t size);

bool CreateImage(VkImage *image, VkDeviceMemory *imageMemory, VkFormat format, VkExtent3D extent,
                        uint8_t mipmapLevels, VkSampleCountFlags samples,
                        VkImageUsageFlags usageFlags, const char *imageType);

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, VkCommandPool commandPool);

bool EndCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue);

bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
                         VkBuffer *buffer, VkDeviceMemory *bufferMemory);

bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CleanupSwapChain();

void CleanupColorImage();

void CleanupDepthImage();

void CleanupPipeline();

void CleanupSyncObjects();
#pragma endregion helperFunctions

#pragma region drawingHelpers
void UpdateUniformBuffer(uint32_t currentFrame);

VkResult BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);

VkResult EndRenderPass(VkCommandBuffer commandBuffer);

void DrawVertexBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VertexBuffer vertexBuffer);
#pragma endregion drawingHelpers

#endif //VULKANHELPERS_H
