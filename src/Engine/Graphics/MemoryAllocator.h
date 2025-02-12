#pragma once
#include "vk_mem_alloc.h"
#include <vector>

namespace Engine::Graphics {
class InstanceManager;

class MemoryAllocator {
private:
  VmaAllocator allocator;
#ifndef NDEBUG
  std::vector<std::tuple<VkImage, uint16_t, const char *>> allocatedImages;
  std::vector<std::tuple<VkBuffer, uint16_t, const char *>> allocatedBuffers;
#endif

public:
  MemoryAllocator() {};
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

  ~MemoryAllocator();

// Allocate memory objects
#ifdef NDEBUG
  void CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const;
#else
private:
  void _CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const;

public:
  void CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation,
                   char const *label = nullptr);
#endif
#ifdef NDEBUG
  void CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                    VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo) const;
#else
private:
  void _CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                     VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo) const;

public:
  void CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                    VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo,
                    char const *label = nullptr);
#endif

  // Free memory objects
  void DestroyImage(VkImage const &image, VmaAllocation const &allocation)
#ifdef NDEBUG
      const
#endif
  {
    allocatedImages.erase(std::remove_if(
        allocatedImages.begin(), allocatedImages.end(),
        [image](std::tuple<VkImage, uint16_t, const char *> &tuple) { return std::get<0>(tuple) == image; }));
    vmaDestroyImage(allocator, image, allocation);
  }
  void DestroyBuffer(VkBuffer const &buffer, VmaAllocation const &allocation)
#ifdef NDEBUG
      const
#endif
  {
    allocatedBuffers.erase(std::remove_if(
        allocatedBuffers.begin(), allocatedBuffers.end(),
        [buffer](std::tuple<VkBuffer, uint16_t, const char *> const &tuple) { return std::get<0>(tuple) == buffer; }));
    vmaDestroyBuffer(allocator, buffer, allocation);
  }
};
} // namespace Engine::Graphics
