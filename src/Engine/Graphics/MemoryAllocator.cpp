#include "MemoryAllocator.h"

#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "Util/Macros.h"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "Debug/Logging.h"


Engine::Graphics::MemoryAllocator::~MemoryAllocator() {
  #ifndef NDEBUG
  for (auto undestroyed : allocatedImages) {
    auto label = std::get<2>(undestroyed);
    if (label) {
      ENGINE_ERROR("Image was not destroyed: {}", label);
    } else {
      ENGINE_ERROR("Image was not destroyed: UNNAMED");
    }
  }
  for (auto undestroyed : allocatedBuffers) {
    auto label = std::get<2>(undestroyed);
    if (label) {
      ENGINE_ERROR("Buffer was not destroyed: {}", label);
    } else {
      ENGINE_ERROR("Buffer was not destroyed: UNNAMED");
    }
  }
  #endif
  vmaDestroyAllocator(allocator);
}

#ifdef NDEBUG
void Engine::Graphics::MemoryAllocator::CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image,
                                                    VmaAllocation *allocation) const
#else
void Engine::Graphics::MemoryAllocator::_CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image,
                                                     VmaAllocation *allocation) const
#endif
{
  VmaAllocationCreateInfo allocationInfo{.usage = VMA_MEMORY_USAGE_GPU_ONLY,
                                         .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
  VULKAN_ASSERT(vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, image, allocation, nullptr),
                "Failed to create image!")
}

#ifndef NDEBUG
void Engine::Graphics::MemoryAllocator::CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image,
                                                    VmaAllocation *allocation, char const *label) {
  _CreateImage(imageCreateInfo, image, allocation);
  VkDebugUtilsObjectNameInfoEXT const nameInfo = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .objectType = VK_OBJECT_TYPE_IMAGE,
    .objectHandle = (uint64_t)*image,
    .pObjectName = label
  };
  
  VULKAN_ASSERT(SetDebugLabel(device, &nameInfo), "Failed to assign debug label to image")
  
  allocatedImages.push_back({*image, static_cast<uint16_t>(allocatedImages.size()), label});
}
#endif

#ifdef NDEBUG
void Engine::Graphics::MemoryAllocator::CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo,
                                                     VmaAllocationCreateInfo const *allocationCreateInfo,
                                                     VkBuffer *buffer, VmaAllocation *allocation,
                                                     VmaAllocationInfo *allocationInfo) const
#else
void Engine::Graphics::MemoryAllocator::_CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo,
                                                      VmaAllocationCreateInfo const *allocationCreateInfo,
                                                      VkBuffer *buffer, VmaAllocation *allocation,
                                                      VmaAllocationInfo *allocationInfo) const
#endif
{
  VULKAN_ASSERT(vmaCreateBuffer(allocator, bufferCreateInfo, allocationCreateInfo, buffer, allocation, allocationInfo),
                "Failed to create buffer!")
}

#ifndef NDEBUG
void Engine::Graphics::MemoryAllocator::CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo,
                                                     VmaAllocationCreateInfo const *allocationCreateInfo,
                                                     VkBuffer *buffer, VmaAllocation *allocation,
                                                     VmaAllocationInfo *allocationInfo, char const *label) {
  _CreateBuffer(bufferCreateInfo, allocationCreateInfo, buffer, allocation, allocationInfo);

  VkDebugUtilsObjectNameInfoEXT const nameInfo = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .objectType = VK_OBJECT_TYPE_BUFFER,
    .objectHandle = (uint64_t)*buffer,
    .pObjectName = label
  };
  
  VULKAN_ASSERT(SetDebugLabel(device, &nameInfo), "Failed to assign debug label to buffer")
  allocatedBuffers.push_back({*buffer, static_cast<uint16_t>(allocatedBuffers.size()), label});
}


// Load function
VkResult Engine::Graphics::MemoryAllocator::SetDebugLabel(VkDevice device, VkDebugUtilsObjectNameInfoEXT const * pNameInfo) const {
  auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
  if (func != nullptr) {
    return func(device, pNameInfo);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
#endif