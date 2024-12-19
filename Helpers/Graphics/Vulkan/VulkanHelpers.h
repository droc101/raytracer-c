//
// Created by Noah on 11/23/2024.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

// It lies.
// ReSharper disable CppUnusedIncludeDirective
#include <cglm/cglm.h>
#include <vulkan/vulkan.h>
#include "../../../Assets/AssetReader.h"
#include "../../../Assets/Assets.h"
#include "../../Core/DataReader.h"
#include "../../Core/Logging.h"
// ReSharper restore CppUnusedIncludeDirective

#pragma region macros
#define VULKAN_VERSION VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)
#define MAX_FRAMES_IN_FLIGHT 2
// TODO Verify start size
#define MAX_UI_QUADS_INIT 2048
#define MAX_WALLS_INIT 100

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
#define GET_COLOR(color) const float r = ((color) >> 16 & 0xFF) / 255.0f; const float g = ((color) >> 8 & 0xFF) / 255.0f; const float b = ((color) & 0xFF) / 255.0f; const float a = ((color) >> 24 & 0xFF) / 255.0f
#define TextureIndex(texture) texturesAssetIDMap[ReadUintA(DecompressAsset(texture), 12)]
#pragma endregion macros

#pragma region typedefs
/**
 * A struct to hold the indicies of the queue families for graphics and presentation.
 * This is used to find and store the indices, which allows for picking between unique and non-unique indices.
 */
typedef struct QueueFamilyIndices
{
    /// The index of the family on the GPU that will be used for graphics processing
    uint32_t graphicsFamily;
    /// The index of the family on the GPU that will be used for presentation
    /// @note If the graphics family supports presentation, @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::graphicsFamily
    /// @note Similarly, if the graphics family does not support presentation then @c QueueFamilyIndices::presentFamily will contain the same value as @c QueueFamilyIndices::uniquePresentFamily
    uint32_t presentFamily;
    /// If the graphics family does not support presentation this will contain the same value as @c QueueFamilyIndices::presentFamily
    uint32_t uniquePresentFamily;
    /// The total count of unique families
    /// @note If this is 1, then @code QueueFamilyIndices::presentFamily == QueueFamilyIndices::graphicsFamily@endcode and @code QueueFamilyIndices::uniquePresentFamily == UINT32_MAX@endcode
    /// @note If this is 2, then @code QueueFamilyIndices::presentFamily != QueueFamilyIndices::graphicsFamily@endcode and @code QueueFamilyIndices::uniquePresentFamily == QueueFamilyIndices::presentFamily@endcode
    uint8_t familyCount;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails
{
    uint32_t formatCount;
    uint32_t presentModeCount;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentMode;
    VkSurfaceCapabilitiesKHR capabilities;
} SwapChainSupportDetails;

/**
 * A struct used to hold information about the size and type of the memory, as well as a host pointer mapped to the
 * Vulkan memory allocation.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceMemory.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlags.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlagBits.html
 */
typedef struct MemoryInfo
{
    /// The size of the block of memory.
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
    VkDeviceSize size;
    /// A pointer to the host memory block mapped to this block of memory.
    void *mappedMemory;
    /// The actual Vulkan memory handle.
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceMemory.html
    VkDeviceMemory memory;
    /// A bitmask where bit n is set if the nth memory type of the @c VkPhysicalDeviceMemoryProperties struct for the
    /// physical device is a supported memory type for the resource.
    uint32_t memoryTypeBits;
    /// A bitmask of VkMemoryPropertyFlagBits that describes the memory type.
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlags.html
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryPropertyFlagBits.html
    VkMemoryPropertyFlags type;
} MemoryInfo;

/**
 * A struct used to hold information about the allocation of a certain resource out of a larger block of memory.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryRequirements.html
 */
typedef struct MemoryAllocationInfo
{
    /// The offset at which the object resides within the larger block of memory.
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkDeviceSize.html
    VkDeviceSize offset;
    /// A pointer to a MemoryInfo struct containing more information about the larger block of memory.
    MemoryInfo *memoryInfo;
    /// Information about the allocation requirements such as size, alignment, and memory type.
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkMemoryRequirements.html
    VkMemoryRequirements memoryRequirements;
} MemoryAllocationInfo;

typedef struct MemoryPools
{
    MemoryInfo localMemory;
    MemoryInfo sharedMemory;
} MemoryPools;

typedef struct UiVertex
{
    vec4 posXY_uvZW;
    vec4 color;
    uint32_t textureIndex;
} UiVertex;

typedef struct WallVertex
{
    float x;
    float y;
    float z;

    float u;
    float v;

    uint32_t textureIndex; // TODO Per-vertex is less than ideal
} WallVertex;

typedef struct Image
{
    VkImage image;
    VkExtent3D extent;
    uint8_t mipmapLevels;
    MemoryAllocationInfo allocationInfo;
} Image;

/**
 * A structure holding data about a Vulkan buffer.
 *
 * This structure is used to keep track of the buffer handle, as well as the memory allocation information associated
 * with the given buffer.
 *
 * @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBuffer.html
 */
typedef struct Buffer
{
    /// The actual Vulkan buffer handle
    /// @see https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkBuffer.html
    VkBuffer buffer;
    /// Stores information about what memory contains the buffer as well as where the buffer is in the memory.
    MemoryAllocationInfo memoryAllocationInfo;
} Buffer;

/**
 * A structure holding data about a translation uniform buffer.
 *
 * This structure is used to keep track of not only the larger buffer that the vertex buffer is offset into, but also to
 * keep track of the host mapped memory and vertex count information.
 *
 * @note This is still C, so there are no actual guardrails preventing you from potentially causing a SEGFAULT by attempting to use @c data as an array.
 */
typedef struct TranslationUniformBuffer
{
    /// The larger buffer within which this uniform buffer resides.
    Buffer *bufferInfo;
    /// The offset into the larger buffer at which the uniform buffer can be found.
    VkDeviceSize offset;
    /// This pointer will be mapped directly to an offset into some larger block of memory, which allows for data to be
    /// written to the uniform buffer directly.
    /// @note This pointer is NOT an array, and only has @c sizeof(mat4) bytes allocated to it.
    mat4 *data;
} TranslationUniformBuffer;

/**
 * A structure holding data about a UI vertex buffer.
 *
 * This structure is used to keep track of not only the larger buffer that the vertex buffer is offset into, but also to
 * keep track of the host mapped memory and vertex count information.
 *
 * @note This is still C, so there are no actual guardrails preventing you from potentially causing a SEGFAULT by attempting to write to @c vertices[100] when @c maxQuads is only 5.
 */
typedef struct UiVertexBuffer
{
    /// The larger buffer within which this vertex buffer resides.
    Buffer *bufferInfo;
    /// The offset into the larger buffer at which this vertex buffer can be found.
    VkDeviceSize verticesOffset;
    /// The offset into the larger buffer at which the UI index buffer can be found.
    VkDeviceSize indicesOffset;
    /// This pointer will be mapped directly to an offset into some larger block of memory.
    /// It is able to be used to directly write up to @code maxQuads * 4@endcode elements of type @c UiVertex to the vertex buffer.
    /// @note This pointer takes the form of @code UiVertex[maxQuads * 4]@endcode.
    UiVertex *vertices;
    /// This pointer will be mapped directly to an offset into some larger block of memory.
    /// It is able to be used to directly write up to @code maxQuads * 4@endcode elements of type @c uint32_t to the index buffer.
    /// @note This pointer takes the form of @code uint32_t[maxQuads * 6]@endcode.
    uint32_t *indices;
    /// A fallback pointer that can be used if it is necessary to write more than @code maxQuads * 4@endcode vertices to the buffer.
    /// @note This pointer takes the form of @code UiVertex[fallbackMaxQuads * 4]@endcode.
    /// @note This pointer is host only, meaning that there is no Vulkan memory backing it. This means that for the GPU to be able to access it the data must first be copied to a buffer.
    UiVertex *fallback;
    /// A fallback pointer that can be used if it is necessary to write more than @code maxQuads * 6@endcode indices to the buffer.
    /// @note This pointer takes the form of @code uint32_t[fallbackMaxVertices * 6]@endcode.
    /// @note This pointer is host only, meaning that there is no Vulkan memory backing it. This means that for the GPU to be able to access it the data must first be copied to a buffer.
    uint32_t *fallbackIndices;
    /// The current number of vertices that are stored in the buffer.
    uint32_t quadCount;
    /// The maximum number of vertices that can be stored in the buffer with the currently allocated memory.
    uint32_t maxQuads;
    /// The maximum number of vertices that can be stored in the fallback buffer with the currently allocated memory.
    uint32_t fallbackMaxQuads;
} UiVertexBuffer;

/**
 * A structure holding data about a wall vertex buffer.
 *
 * This structure is used to keep track of the larger buffer that the buffer is a part of, offset information,
 * and information about the number of walls.
 */
typedef struct WallVertexBuffer
{
    /// The larger buffer within which the wall vertex buffer reside.
    Buffer *bufferInfo;
    /// The offset of the wall vertex buffer into the larger buffer allocation.
    VkDeviceSize verticesOffset;
    /// The offset of the index buffer for the wall vertex buffer into the larger buffer allocation.
    VkDeviceSize indicesOffset;
    /// The number of walls that are currently stored in the buffer.
    uint32_t wallCount;
    /// The maximum number of walls that can currently be stored in the buffer.
    uint32_t maxWallCount;
} WallVertexBuffer;

typedef struct Buffers
{
    Buffer local;
    Buffer shared;

    UiVertexBuffer ui;
    WallVertexBuffer walls;
    TranslationUniformBuffer translation[MAX_FRAMES_IN_FLIGHT];
} Buffers;

typedef struct Pipelines
{
    VkPipeline walls;
    VkPipeline ui;
} Pipelines;

typedef struct TextureSamplers
{
    VkSampler linearRepeat;
    VkSampler nearestRepeat;
    VkSampler linearNoRepeat;
    VkSampler nearestNoRepeat;
} TextureSamplers;

typedef struct PhysicalDevice
{
    /// The physical device is the hardware available to the host that has an implementation of Vulkan.
    /// @see https://docs.vulkan.org/spec/latest/chapters/devsandqueues.html#devsandqueues-physical-device-enumeration
    /// @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html
    VkPhysicalDevice device;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
} PhysicalDevice;
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
/// @todo Document this along with the struct
extern PhysicalDevice physicalDevice;
/// @todo Document this
extern QueueFamilyIndices queueFamilyIndices;
/// @todo Document this along with the struct
extern SwapChainSupportDetails swapChainSupport;
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
extern VkPipelineCache pipelineCache;
extern Pipelines pipelines;
extern VkFramebuffer *swapChainFramebuffers;
extern VkCommandPool graphicsCommandPool;
extern VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
extern VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
extern bool framebufferResized;
extern uint8_t currentFrame;
extern uint32_t swapchainImageIndex;
extern MemoryPools memoryPools;
extern Buffers buffers;
extern VkDescriptorPool descriptorPool;
extern VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
extern Image textures[TEXTURE_ASSET_COUNT];
extern VkDeviceMemory textureMemory;
extern VkImageView texturesImageView[TEXTURE_ASSET_COUNT];
extern uint32_t texturesAssetIDMap[ASSET_COUNT];
extern TextureSamplers textureSamplers;
extern VkFormat depthImageFormat;
extern VkImage depthImage;
extern VkDeviceMemory depthImageMemory;
extern VkImageView depthImageView;
extern VkImage colorImage;
extern VkDeviceMemory colorImageMemory;
extern VkImageView colorImageView;
extern VkClearColorValue clearColor;
extern VkSampleCountFlagBits msaaSamples;
#pragma endregion variables

#pragma region helperFunctions
/**
 * Provides information about the physical device's support for the swap chain.
 * @param pDevice The physical device to query for
 * @return A @c SwapChainSupportDetails struct
 */
bool QuerySwapChainSupport(VkPhysicalDevice pDevice);

bool CreateImageView(VkImageView *imageView,
                     VkImage image,
                     VkFormat format,
                     VkImageAspectFlagBits aspectMask,
                     uint8_t mipmapLevels,
                     const char *errorMessage);

VkShaderModule CreateShaderModule(const uint32_t *code, size_t size);

bool CreateImage(VkImage *image,
                 VkDeviceMemory *imageMemory,
                 VkFormat format,
                 VkExtent3D extent,
                 uint8_t mipmapLevels,
                 VkSampleCountFlags samples,
                 VkImageUsageFlags usageFlags,
                 const char *imageType);

bool BeginCommandBuffer(const VkCommandBuffer *commandBuffer, VkCommandPool commandPool);

bool EndCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue);

bool CreateBuffer(VkBuffer *buffer,
                  VkDeviceSize size,
                  VkBufferUsageFlags usageFlags,
                  bool newAllocation,
                  MemoryAllocationInfo *allocationInfo);

bool CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CleanupSwapChain();

void CleanupColorImage();

void CleanupDepthImage();

void CleanupPipeline();

void CleanupSyncObjects();
#pragma endregion helperFunctions

#pragma region drawingHelpers
void UpdateUniformBuffer(const Camera *camera, uint32_t currentFrame);

VkResult BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);

VkResult EndRenderPass(VkCommandBuffer commandBuffer);

bool DrawRectInternal(float ndcStartX,
                      float ndcStartY,
                      float ndcEndX,
                      float ndcEndY,
                      float startU,
                      float startV,
                      float endU,
                      float endV,
                      uint32_t color,
                      uint32_t textureIndex);

bool DrawQuadInternal(const mat4 vertices_posXY_uvZW,
                      uint32_t color,
                      uint32_t textureIndex);
#pragma endregion drawingHelpers

#endif //VULKANHELPERS_H
