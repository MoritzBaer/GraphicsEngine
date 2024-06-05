#pragma once
#include "Util/DeletionQueue.h"
#include "vk_mem_alloc.h"
#include <vector>

namespace Engine::Graphics {
class MemoryAllocator : public ConstDestroyable {
private:
  VmaAllocator allocator;
#ifndef NDEBUG
  std::vector<std::tuple<VkImage, uint16_t>> allocatedImages;
  std::vector<std::tuple<VkBuffer, uint16_t>> allocatedBuffers;
#endif

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
  void CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation)
#ifdef NDEBUG
      const
#endif
      ;
  void CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                    VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo)
#ifdef NDEBUG
      const
#endif
      ;

  // Free memory objects
  void DestroyImage(VkImage const &image, VmaAllocation const &allocation)
#ifdef NDEBUG
      const
#endif
  {
    allocatedImages.erase(
        std::remove_if(allocatedImages.begin(), allocatedImages.end(),
                       [image](std::tuple<VkImage, uint16_t> &tuple) { return std::get<0>(tuple) == image; }));
    vmaDestroyImage(allocator, image, allocation);
  }
  void DestroyBuffer(VkBuffer const &buffer, VmaAllocation const &allocation)
#ifdef NDEBUG
      const
#endif
  {
    allocatedBuffers.erase(
        std::remove_if(allocatedBuffers.begin(), allocatedBuffers.end(),
                       [buffer](std::tuple<VkBuffer, uint16_t> const &tuple) { return std::get<0>(tuple) == buffer; }));
    vmaDestroyBuffer(allocator, buffer, allocation);
  }
};

inline MemoryAllocator mainAllocator;
} // namespace Engine::Graphics
