#pragma once

#include "vulkan/vulkan.h"
#include <vector>

namespace Engine::Graphics
{
    class Command {
    public:
        virtual void QueueExecution(VkCommandBuffer const & queue) const = 0;
    };

    class CommandQueue
    {
    private:
        VkCommandPool commandPool;
        VkCommandBuffer mainBuffer;

    public:
        void Create();
        void Destroy();

        VkCommandBufferSubmitInfo EnqueueCommandSequence(std::initializer_list<Command const *> commands, VkCommandBufferUsageFlags flags = 0) const;
    };

    class PipelineBarrierCommand : public Command {
        std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;
    public: 
        PipelineBarrierCommand(std::vector<VkImageMemoryBarrier2> const & imageMemoryBarriers);
        void QueueExecution(VkCommandBuffer const & queue) const;
    };

    class ClearColourCommand : public Command {
        VkImage image;
        VkImageLayout currentLayout;
        VkClearColorValue clearColour;
        std::vector<VkImageSubresourceRange> subresourceRanges;
    public:
        ClearColourCommand(VkImage image, VkImageLayout currentLayout, VkClearColorValue const & clearValue, std::vector<VkImageSubresourceRange> const & subresourceRanges);
        void QueueExecution(VkCommandBuffer const & queue) const;
    };
} // namespace Engine::Graphics
