#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "InstanceManager.h"
#include "CommandQueue.h"

namespace Engine::Graphics
{
    class Buffer {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;

    public:
        void Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        void Destroy();

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
        inline size_t Size() const { return info.size; }

        class BufferCopyCommand : public Command {
            VkBuffer src;
            VkBuffer dst;
            size_t srcOffset;
            size_t dstOffset;
            size_t size;
        public: 
            BufferCopyCommand(VkBuffer source, VkBuffer destination, size_t size, size_t sourceOffset = 0, size_t destinationOffset = 0)
                : src(source), dst(destination), srcOffset(sourceOffset), dstOffset(destinationOffset), size(size) { }
            void QueueExecution(VkCommandBuffer const & queue) const;
        } CopyTo(Buffer const & other, size_t size, size_t sourceOffset = 0, size_t destinationOffset = 0) const;

        inline class BindBufferAsIndexBufferCommand : public Command {
            VkBuffer buffer;
        public: 
            BindBufferAsIndexBufferCommand(VkBuffer const & indexBuffer) : buffer(indexBuffer) { }
            inline void QueueExecution(VkCommandBuffer const & queue) const { vkCmdBindIndexBuffer(queue, buffer, 0, VK_INDEX_TYPE_UINT32); }
        } BindAsIndexBuffer() const { return BindBufferAsIndexBufferCommand(buffer); }
    };
} // namespace Engine::Graphics
