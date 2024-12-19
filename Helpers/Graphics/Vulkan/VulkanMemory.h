//
// Created by Noah on 12/18/2024.
//

#ifndef VULKANMEMORY_H
#define VULKANMEMORY_H

#include <stdbool.h>

bool AllocateLocalMemory();
bool BindLocalMemory();
bool CreateLocalMemory();

bool AllocateSharedMemory();
bool BindSharedMemory();
bool MapSharedMemory();
bool CreateSharedMemory();

#endif //VULKANMEMORY_H
