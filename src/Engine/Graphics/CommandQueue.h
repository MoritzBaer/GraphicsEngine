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

    class BlitImageCommand : public Command {
        std::vector<VkImageBlit2> blitRegions;
        VkImage source, destination;
    public:
        BlitImageCommand(VkImage const & source, VkImage const & destination, std::vector<VkImageBlit2> const & blitRegions) : 
            blitRegions(blitRegions), source(source), destination(destination) { }
        void QueueExecution(VkCommandBuffer const & queue) const;
    };

    class BindPipelineCommand : public Command {
        VkPipelineBindPoint bindPoint;
        VkPipeline pipeline;
    public:
        BindPipelineCommand(VkPipeline const & pipeline, VkPipelineBindPoint const & bindPoint) : pipeline(pipeline), bindPoint(bindPoint) { }
        inline void QueueExecution(VkCommandBuffer const & queue) const { vkCmdBindPipeline(queue, bindPoint, pipeline); }
    };

    class BindDescriptorSetsCommand : public Command {
        VkPipelineBindPoint bindPoint;
        VkPipelineLayout layout;
        std::vector<VkDescriptorSet> descriptors;
    public:
        BindDescriptorSetsCommand(VkPipelineBindPoint const & bindPoint, VkPipelineLayout const & layout, std::vector<VkDescriptorSet> const & descriptors) :
            bindPoint(bindPoint), layout(layout), descriptors(descriptors) { }
        BindDescriptorSetsCommand(VkPipelineBindPoint const & bindPoint, VkPipelineLayout const & layout, VkDescriptorSet const & descriptor) :
            bindPoint(bindPoint), layout(layout), descriptors{descriptor} { }
        inline void QueueExecution(VkCommandBuffer const & queue) const { vkCmdBindDescriptorSets(queue, bindPoint, layout, 0, static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr); }
    };

    class DispatchCommand : public Command {
        uint32_t gx, gy, gz;
    public: 
        DispatchCommand(uint32_t workerGroupsX, uint32_t workerGroupsY, uint32_t workerGroupsZ) : gx(workerGroupsX), gy(workerGroupsY), gz(workerGroupsZ) { }
        inline void QueueExecution(VkCommandBuffer const & queue) const { vkCmdDispatch(queue, gx, gy, gz); }
    };

    class ExecuteComputePipelineCommand : public Command {
        BindPipelineCommand bindPipeline;
        BindDescriptorSetsCommand bindDescriptors;
        DispatchCommand dispatch;
    public: 
        ExecuteComputePipelineCommand(
                VkPipeline const & pipeline, 
                VkPipelineBindPoint const & bindPoint,
                VkPipelineLayout const & layout, 
                std::vector<VkDescriptorSet> const & descriptors,
                uint32_t workerGroupsX, 
                uint32_t workerGroupsY, 
                uint32_t workerGroupsZ
            ) :
                bindPipeline(pipeline, bindPoint),
                bindDescriptors(bindPoint, layout, descriptors),
                dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) { }
        ExecuteComputePipelineCommand(
                VkPipeline const & pipeline, 
                VkPipelineBindPoint const & bindPoint,
                VkPipelineLayout const & layout, 
                VkDescriptorSet const & descriptor,
                uint32_t workerGroupsX, 
                uint32_t workerGroupsY, 
                uint32_t workerGroupsZ
            ) :
                bindPipeline(pipeline, bindPoint),
                bindDescriptors(bindPoint, layout, descriptor),
                dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) { }
        void QueueExecution(VkCommandBuffer const & queue) const;
    };

} // namespace Engine::Graphics
