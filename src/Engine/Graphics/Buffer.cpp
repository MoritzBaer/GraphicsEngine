#include "Buffer.h"
#include "MemoryAllocator.h"

void Engine::Graphics::Buffer::Create(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage
    };

    VmaAllocationCreateInfo allocInfo {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = memoryUsage
    };

    mainAllocator.CreateBuffer(&bufferInfo, &allocInfo, &buffer, &allocation, &info);
}

void Engine::Graphics::Buffer::Destroy()
{
    mainAllocator.DestroyBuffer(buffer, allocation);
}

Engine::Graphics::Buffer::BufferCopyCommand Engine::Graphics::Buffer::CopyTo(Buffer const &other, size_t size, size_t sourceOffset, size_t destinationOffset) const
{
    return BufferCopyCommand(buffer, other.buffer, size, sourceOffset, destinationOffset);
}

void Engine::Graphics::Buffer::BufferCopyCommand::QueueExecution(VkCommandBuffer const & queue) const
{
    VkBufferCopy copy {
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = size
    };

    vkCmdCopyBuffer(queue, src, dst, 1, &copy);
}