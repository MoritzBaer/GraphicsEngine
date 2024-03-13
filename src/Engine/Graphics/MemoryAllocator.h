#pragma once
#include "vk_mem_alloc.h"
#include "Util/DeletionQueue.h"

namespace Engine::Graphics
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

        // TODO: Maybe switch for methods creating Image/Buffer objects
        // Allocate memory objects
        void CreateImage(VkImageCreateInfo const * imageCreateInfo, VkImage * image, VmaAllocation * allocation) const;
        void CreateBuffer(VkBufferCreateInfo const * bufferCreateInfo, VmaAllocationCreateInfo const * allocationCreateInfo, VkBuffer * buffer, VmaAllocation * allocation, VmaAllocationInfo * allocationInfo) const;

        // Free memory objects
        inline void DestroyImage(VkImage const & image, VmaAllocation const & allocation) const { vmaDestroyImage(allocator, image, allocation); }
        inline void DestroyBuffer(VkBuffer const & buffer, VmaAllocation const & allocation) const { vmaDestroyBuffer(allocator, buffer, allocation); }
    };

    inline MemoryAllocator mainAllocator;
} // namespace Engine
