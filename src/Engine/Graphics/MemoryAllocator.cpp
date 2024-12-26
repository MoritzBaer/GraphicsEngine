#include "MemoryAllocator.h"

#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "Util/Macros.h"

Engine::Graphics::MemoryAllocator::MemoryAllocator(InstanceManager const &instanceManager) {
  instanceManager.CreateMemoryAllocator(*this);
}

Engine::Graphics::MemoryAllocator::~MemoryAllocator() {
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
  allocatedBuffers.push_back({*buffer, static_cast<uint16_t>(allocatedBuffers.size()), label});
}
#endif