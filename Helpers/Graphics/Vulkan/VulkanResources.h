//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANRESOURCES_H
#define VULKANRESOURCES_H

#include <stdbool.h>

bool CreateLocalBuffer();
bool SetLocalBufferAliasingInfo();

bool CreateSharedBuffer();
bool SetSharedBufferAliasingInfo();

void UpdateDescriptorSets();

#endif //VULKANRESOURCES_H
