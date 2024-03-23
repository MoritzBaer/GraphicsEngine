#pragma once
#include "Util/DeletionQueue.h"
#include "vk_mem_alloc.h"

namespace Engine::Graphics {
class MemoryAllocator : public Destroyable {
private:
  VmaAllocator allocator;

public:
  inline void Create(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkInstance instance) {
    VmaAllocatorCreateInfo allocatorInfo{
#ifndef COMPILE_FOR_RENDERDOC
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
#endif
        .physicalDevice = physicalDevice,
        .device = logicalDevice,
        .instance = instance,
    };

    vmaCreateAllocator(&allocatorInfo, &allocator);
  }

  inline void Destroy() const { vmaDestroyAllocator(allocator); }

  // Allocate memory objects
  void CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const;
  void CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                    VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo) const;

  // Free memory objects
  void DestroyImage(VkImage const &image, VmaAllocation const &allocation) const {
    vmaDestroyImage(allocator, image, allocation);
  }
  void DestroyBuffer(VkBuffer const &buffer, VmaAllocation const &allocation) const {
    vmaDestroyBuffer(allocator, buffer, allocation);
  }
};

inline MemoryAllocator mainAllocator;
} // namespace Engine::Graphics
