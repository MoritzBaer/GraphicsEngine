#include "MemoryAllocator.h"

#include "../Macros.h"
#include "../Debug/Logging.h"

void Engine::MemoryAllocator::CreateImage(VkImageCreateInfo const *imageCreateInfo, VkImage *image, VmaAllocation *allocation) const {
    VmaAllocationCreateInfo allocationInfo {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    VULKAN_ASSERT(vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, image, allocation, nullptr), "Failed to create image!")
}