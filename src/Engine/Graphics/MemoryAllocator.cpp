#include "MemoryAllocator.h"

#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "Util/Macros.h"
#include <cstdint>
#include <source_location>
#include <vulkan/vulkan_core.h>

Engine::Graphics::MemoryAllocator::~MemoryAllocator() {
  ENGINE_DEBUG("Deallocating memory manager")
#ifndef NDEBUG
  for (auto undestroyed : allocatedImages) {
    auto label = std::get<2>(undestroyed);

    ENGINE_ERROR("Image was not destroyed: {} ({})", static_cast<void *>(std::get<0>(undestroyed)),
                 label.empty() ? "unnamed" : label);
  }
  for (auto undestroyed : allocatedBuffers) {
    auto label = std::get<2>(undestroyed);
    ENGINE_ERROR("Buffer was not destroyed: {} ({})", static_cast<void *>(std::get<0>(undestroyed)),
                 label.empty() ? "unnamed" : label);
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
                                                    VmaAllocation *allocation, std::source_location srcLoc) {
  _CreateImage(imageCreateInfo, image, allocation);
  VkDebugUtilsObjectNameInfoEXT const nameInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                                  .objectType = VK_OBJECT_TYPE_IMAGE,
                                                  .objectHandle = (uint64_t)*image,
                                                  .pObjectName = __debug_label};

  VULKAN_ASSERT(SetDebugLabel(device, &nameInfo), "Failed to assign debug label to image")

  allocatedImages.push_back({*image, static_cast<uint16_t>(allocatedImages.size()),
                             std::format("{} ({}:{})", __debug_label, srcLoc.file_name(), srcLoc.line())});
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
                                                     VmaAllocationInfo *allocationInfo, std::source_location srcLoc) {
  _CreateBuffer(bufferCreateInfo, allocationCreateInfo, buffer, allocation, allocationInfo);

  VkDebugUtilsObjectNameInfoEXT const nameInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                                  .objectType = VK_OBJECT_TYPE_BUFFER,
                                                  .objectHandle = (uint64_t)*buffer,
                                                  .pObjectName = __debug_label};

  VULKAN_ASSERT(SetDebugLabel(device, &nameInfo), "Failed to assign debug label to buffer")
  allocatedBuffers.push_back({*buffer, static_cast<uint16_t>(allocatedBuffers.size()),
                              std::format("{} ({}:{})", __debug_label, srcLoc.file_name(), srcLoc.line())});
}

// Load function
VkResult Engine::Graphics::MemoryAllocator::SetDebugLabel(VkDevice device,
                                                          VkDebugUtilsObjectNameInfoEXT const *pNameInfo) const {
  auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
  if (func != nullptr) {
    return func(device, pNameInfo);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
#endif