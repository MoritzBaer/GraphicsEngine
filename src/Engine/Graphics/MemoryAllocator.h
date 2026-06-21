#pragma once

#include "vk_mem_alloc.h"
#include <algorithm>
#include <string>
#include <vector>

#ifdef NDEBUG
#define RELEASE_CONST const
#else
#define RELEASE_CONST
#endif

#ifndef NDEBUG
#include <source_location>
#endif

namespace Engine::Graphics {
#ifndef NDEBUG
#include <source_location>

inline thread_local char const *__debug_label = "UNNAMED";

struct __label_scope {
  __label_scope(char const *lbl) { __debug_label = lbl; }
  ~__label_scope() { __debug_label = "UNNAMED"; }
};

#define DEBUG_LABEL(lbl) __label_scope(lbl),
#define DEBUG_SOURCE_LOCATION_DECLARATION , std::source_location srcLoc = std::source_location::current()
#define DEBUG_SOURCE_LOCATION_REFERENCE , std::source_location srcLoc
#define DEBUG_SOURCE_LOCATION_FORWARD , srcLoc

#else
#define DEBUG_LABEL(lbl)
#define DEBUG_SOURCE_LOCATION_DECLARATION
#define DEBUG_SOURCE_LOCATION_REFERENCE
#define DEBUG_SOURCE_LOCATION_FORWARD
#endif

class InstanceManager;

class MemoryAllocator {
private:
  VmaAllocator allocator;
#ifndef NDEBUG
  std::vector<std::tuple<VkImage, uint16_t, std::string>> allocatedImages;
  std::vector<std::tuple<VkBuffer, uint16_t, std::string>> allocatedBuffers;
  VkDevice device;
  VkInstance instance;
  #endif
  
  private:
    void _CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const;
  public:
  private:
    void _CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                       VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo) const;
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

#ifndef NDEBUG
    device = logicalDevice;
    this->instance = instance;
#endif

    vmaCreateAllocator(&allocatorInfo, &allocator);
  }

  ~MemoryAllocator();

// Allocate memory objects
void CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation DEBUG_SOURCE_LOCATION_DECLARATION) RELEASE_CONST;

void CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo,
                  VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo DEBUG_SOURCE_LOCATION_DECLARATION) RELEASE_CONST;

  // Free memory objects
  void DestroyImage(VkImage const &image, VmaAllocation const &allocation) RELEASE_CONST {
#ifndef NDEBUG
    allocatedImages.erase(std::remove_if(
        allocatedImages.begin(), allocatedImages.end(),
        [image](std::tuple<VkImage, uint16_t, std::string> &tuple) { return std::get<0>(tuple) == image; }));
#endif
    vmaDestroyImage(allocator, image, allocation);
  }
  void DestroyBuffer(VkBuffer const &buffer, VmaAllocation const &allocation) RELEASE_CONST {
#ifndef NDEBUG
    allocatedBuffers.erase(std::remove_if(
        allocatedBuffers.begin(), allocatedBuffers.end(),
        [buffer](std::tuple<VkBuffer, uint16_t, std::string> const &tuple) { return std::get<0>(tuple) == buffer; }));
#endif
    vmaDestroyBuffer(allocator, buffer, allocation);
  }

#ifndef NDEBUG
  // Set debug labels
  VkResult SetDebugLabel(VkDevice device, VkDebugUtilsObjectNameInfoEXT const *pNameInfo) const;
#endif
};
} // namespace Engine::Graphics
