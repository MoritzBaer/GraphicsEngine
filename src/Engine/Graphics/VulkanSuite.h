#pragma once

#include "Graphics/GPUObjectManager.h"

namespace Engine::Graphics {
struct VulkanSuite {
  InstanceManager instanceManager;
  MemoryAllocator memoryAllocator;
  GPUObjectManager gpuObjectManager;

  VulkanSuite(const char *appName, Window const *window)
      : instanceManager(appName, window), memoryAllocator(), gpuObjectManager(&instanceManager, &memoryAllocator) {
    instanceManager.CreateMemoryAllocator(memoryAllocator);
  }
};
} // namespace Engine::Graphics
