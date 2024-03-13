#include "MemoryAllocator.h"

#include "Util/Macros.h"
#include "Debug/Logging.h"

void Engine::Graphics::MemoryAllocator::CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const {
    VmaAllocationCreateInfo allocationInfo {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    VULKAN_ASSERT(vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, image, allocation, nullptr), "Failed to create image!")
}

void Engine::Graphics::MemoryAllocator::CreateBuffer(VkBufferCreateInfo const *bufferCreateInfo, VmaAllocationCreateInfo const *allocationCreateInfo, VkBuffer *buffer, VmaAllocation *allocation, VmaAllocationInfo *allocationInfo) const
{
    VULKAN_ASSERT(vmaCreateBuffer(allocator, bufferCreateInfo, allocationCreateInfo, buffer, allocation, allocationInfo), "Failed to create buffer!")
}
