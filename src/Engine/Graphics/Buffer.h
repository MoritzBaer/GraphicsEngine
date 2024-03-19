#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "InstanceManager.h"
#include "CommandQueue.h"
#include "MemoryAllocator.h"
#include "Util/DeletionQueue.h"

namespace Engine::Graphics
{
    class BufferCopyCommand : public Command {
        VkBuffer src;
        VkBuffer dst;
        size_t srcOffset;   // In bytes
        size_t dstOffset;   // In bytes
        size_t size;        // In bytes
    public: 
        BufferCopyCommand(VkBuffer source, VkBuffer destination, size_t size, size_t sourceOffset = 0, size_t destinationOffset = 0)
            : src(source), dst(destination), srcOffset(sourceOffset), dstOffset(destinationOffset), size(size) { }
        void QueueExecution(VkCommandBuffer const & queue) const;
    };

    class BindBufferAsIndexBufferCommand : public Command {
        VkBuffer buffer;
    public: 
        BindBufferAsIndexBufferCommand(VkBuffer const & indexBuffer) : buffer(indexBuffer) { }
        inline void QueueExecution(VkCommandBuffer const & queue) const { vkCmdBindIndexBuffer(queue, buffer, 0, VK_INDEX_TYPE_UINT32); }
    };

    template <typename T>
    class Buffer : public Destroyable {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
        size_t size;

        template <typename T_Other> friend class Buffer;

    public:
        void Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void Destroy() const;

        Buffer() { }
        Buffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) { Create(size, usage, memoryUsage); }

        inline VkDeviceAddress GetDeviceAddresss() const {
            VkBufferDeviceAddressInfo deviceAdressInfo {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = buffer
            };

            return InstanceManager::GetBufferDeviceAddress(&deviceAdressInfo);
        }

        inline void * GetMappedData() const { return info.pMappedData; }
        inline size_t Size() const { return size; }
        inline size_t PhysicalSize() const { return size * sizeof(T); }

        template <typename T_Other>
        class BufferCopyCommand CopyTo(Buffer<T_Other> const & other, size_t size, size_t sourceOffset = 0, size_t destinationOffset = 0) const;

        inline void BindAsIndexBuffer(VkCommandBuffer const & commandBuffer) const requires(std::integral<T>) { vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32); } // TODO: Determine type via switch on T
        
    };

    // Implementations
    template <typename T>
    template <typename T_Other>
    BufferCopyCommand Buffer<T>::CopyTo(Buffer<T_Other> const &other, size_t size, size_t sourceOffset, size_t destinationOffset) const
    {
        return BufferCopyCommand(buffer, other.buffer, size * sizeof(T), sourceOffset * sizeof(T), destinationOffset * sizeof(T_Other));
    }

    template <typename T>
    void Buffer<T>::Destroy() const
    {
        mainAllocator.DestroyBuffer(buffer, allocation);
    }

    template <typename T>
    void Buffer<T>::Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        this->size = size;
        
        VkBufferCreateInfo bufferInfo {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size * sizeof(T),
            .usage = usage
        };

        VmaAllocationCreateInfo allocInfo {
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = memoryUsage
        };

        mainAllocator.CreateBuffer(&bufferInfo, &allocInfo, &buffer, &allocation, &info);
    }

    // Implementations of commands
    inline void BufferCopyCommand::QueueExecution(VkCommandBuffer const &queue) const
    {
        VkBufferCopy copy {
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = size
        };

        vkCmdCopyBuffer(queue, src, dst, 1, &copy);
    }


} // namespace Engine::Graphics
