//
// Created by Noah on 11/9/2024.
//

// ReSharper disable CppUnusedIncludeDirective
#ifndef VULKANINTERNAL_H
#define VULKANINTERNAL_H

#include <cglm/cglm.h>

#pragma region internalFunctions
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

bool LoadTextures();

bool CreateTexturesImageView();

bool CreateTextureSampler();

bool CreateBuffers();

bool AllocateMemory();

bool CreateDescriptorPool();

bool CreateDescriptorSets();

bool CreateCommandBuffers();

bool CreateSyncObjects();
#pragma endregion internalFunctions

bool RecreateSwapChain();

#endif //VULKANINTERNAL_H
