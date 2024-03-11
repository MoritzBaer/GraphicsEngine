#pragma once
#include "vk_mem_alloc.h"
#include "Util/DeletionQueue.h"

namespace Engine
{
    class MemoryAllocator : public Destroyable
    {
    private:
        VmaAllocator allocator;
    public:
        inline void Create(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkInstance instance) {
            VmaAllocatorCreateInfo allocatorInfo {
#ifndef COMPILE_FOR_RENDERDOC
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
#endif
                .physicalDevice = physicalDevice,
                .device = logicalDevice,
                .instance = instance,
            };

            vmaCreateAllocator(&allocatorInfo, &allocator);
        }

        inline void Destroy() { vmaDestroyAllocator(allocator); }

        void CreateImage(VkImageCreateInfo const * imageCreateInfo, VkImage * image, VmaAllocation * allocation) const;

        inline void DestroyImage(VkImage & image, VmaAllocation &allocation) const { vmaDestroyImage(allocator, image, allocation); }
    };

    inline MemoryAllocator mainAllocator;
} // namespace Engine
