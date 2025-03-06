//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <stdbool.h>
#include "VulkanHelpers.h"

bool CreateLocalBuffer();

void SetLocalBufferAliasingInfo();

/**
 * This function is used to create the buffer that lives in memory that is accessible by both the GPU and the CPU.
 * This function creates the handle to the buffer and allocates the memory for UI vertices and indices.
 *
 * This function expects @c buffers.ui.maxStagingQuads to be set to the maximum number of quads that we should allocate
 * space for. In addition to expecting @c buffers.ui.maxStagingQuads to be set, this function also expects that
 * @c buffers.ui.maxQuads be set to the number of quads that we should allocate space for when allocating the
 * host-exclusive pointer for vertex and index data, with the special case that if it is zero, then the value of
 * @c buffers.ui.maxStagingQuads will be used.
 *
 * @return @c false if any part of the function fails; otherwise @c true
 *
 * @note This function is NOT for recreating or resizing buffers. That functionality can instead be found in the @c ResizeBuffer function. This means that this function expects that @c buffers.shared.buffer is @c NULL, and will do nothing if this is not the case.
 */
bool CreateSharedBuffer();

void SetSharedBufferAliasingInfo();

bool ResizeBuffer(Buffer *buffer, bool lossy);

bool ResizeBufferRegion(Buffer *buffer, VkDeviceSize offset, VkDeviceSize oldSize, VkDeviceSize newSize, bool lossy);

VkResult ResizeUiBuffer();

bool ResizeActorBuffer();

bool LoadTexture(const Image *image);

#endif //VULKANRESOURCES_H
