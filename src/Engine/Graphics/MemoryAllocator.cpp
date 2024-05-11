#include "MemoryAllocator.h"

#include "Debug/Logging.h"
#include "Util/Macros.h"

void Engine::Graphics::MemoryAllocator::CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image,
                                                    VmaAllocation *allocation)
#ifdef NDEBUG
    const
#endif
{
  VmaAllocationCreateInfo allocationInfo{.usage = VMA_MEMORY_USAGE_GPU_ONLY,
                                         .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
  VULKAN_ASSERT(vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, image, allocation, nullptr),
                "Failed to create image!")
#ifndef NDEBUG
  allocatedImages.push_back({*image, static_cast<uint32_t>(allocatedImages.size())});
#endif
}

void Engine::Graphics::MemoryAllocator::CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo,
                                                     VmaAllocationCreateInfo const *allocationCreateInfo,
                                                     VkBuffer *buffer, VmaAllocation *allocation,
                                                     VmaAllocationInfo *allocationInfo)
#ifdef NDEBUG
    const
#endif
{
  VULKAN_ASSERT(vmaCreateBuffer(allocator, bufferCreateInfo, allocationCreateInfo, buffer, allocation, allocationInfo),
                "Failed to create buffer!")
#ifndef NDEBUG
  allocatedBuffers.push_back({*buffer, static_cast<uint32_t>(allocatedBuffers.size())});
#endif
}
