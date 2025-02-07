//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective
#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <cglm/cglm.h>

/**
 * This function will create the Vulkan instance, set up for SDL.
 * @see instance
 */
bool CreateInstance();

/**
 * Creates the Vulkan surface
 * @see surface
 */
bool CreateSurface();

/**
 * This function selects the GPU that will be used to render the game.
 * Assuming I did it right, it will pick the best GPU available.
 */
bool PickPhysicalDevice();

// TODO Use multiple queues from queue families, at least when sub-optimal family setups are in use
bool CreateLogicalDevice();

bool CreateSwapChain();

bool CreateImageViews();

bool CreateRenderPass();

bool CreateDescriptorSetLayouts();

bool CreateGraphicsPipelineCache();

bool CreateGraphicsPipelines();

bool CreateCommandPools();

bool CreateColorImage();

bool CreateDepthImage();

bool CreateFramebuffers();

/// TODO: This will first be written using a separate allocation from the walls and other things that use VRAM, but
///  then once the code is fully working and committed, I will come back to this and add the textures to the shared
///  allocation to improve performance.
bool InitTextures();

bool CreateTexturesImageView();

bool CreateTextureSampler();

bool CreateBuffers();

bool AllocateMemoryPools();

bool CreateDescriptorPool();

bool CreateDescriptorSets();

bool CreateCommandBuffers();

bool CreateSyncObjects();

#endif //VULKANINTERNAL_H
