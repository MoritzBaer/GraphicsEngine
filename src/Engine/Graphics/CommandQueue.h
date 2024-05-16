#pragma once

#include "ComputeEffect.h"
#include "Util/DeletionQueue.h"
#include "vulkan/vulkan.h"
#include <vector>

namespace Engine::Graphics {
class Command {
public:
  virtual void QueueExecution(VkCommandBuffer const &queue) const = 0;
};

class CommandQueue : public ConstDestroyable {
private:
  VkCommandPool commandPool;
  VkCommandBuffer mainBuffer;

public:
  void Create();
  void Destroy() const;

  VkCommandBufferSubmitInfo EnqueueCommandSequence(std::initializer_list<Command const *> commands,
                                                   VkCommandBufferUsageFlags flags = 0) const;
};

class CompositeCommand : public Command {
  std::vector<Command const *> commands;

public:
  CompositeCommand(std::initializer_list<Command const *> const &commands) : commands(commands) {}
  void QueueExecution(VkCommandBuffer const &queue) const {
    for (Command const *command : commands) {
      command->QueueExecution(queue);
    }
  }
};

class PipelineBarrierCommand : public Command {
  std::vector<VkImageMemoryBarrier2> imageMemoryBarriers;

public:
  PipelineBarrierCommand(std::vector<VkImageMemoryBarrier2> const &imageMemoryBarriers);
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class ClearColourCommand : public Command {
  VkImage image;
  VkImageLayout currentLayout;
  VkClearColorValue clearColour;
  std::vector<VkImageSubresourceRange> subresourceRanges;

public:
  ClearColourCommand(VkImage image, VkImageLayout currentLayout, VkClearColorValue const &clearValue,
                     std::vector<VkImageSubresourceRange> const &subresourceRanges);
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class BlitImageCommand : public Command {
  std::vector<VkImageBlit2> blitRegions;
  VkImage source, destination;

public:
  BlitImageCommand(VkImage const &source, VkImage const &destination, std::vector<VkImageBlit2> const &blitRegions)
      : blitRegions(blitRegions), source(source), destination(destination) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

class BindPipelineCommand : public Command {
  VkPipelineBindPoint bindPoint;
  VkPipeline pipeline;

public:
  BindPipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint)
      : pipeline(pipeline), bindPoint(bindPoint) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const { vkCmdBindPipeline(queue, bindPoint, pipeline); }
};

class BindDescriptorSetsCommand : public Command {
  VkPipelineBindPoint bindPoint;
  VkPipelineLayout layout;
  std::vector<VkDescriptorSet> descriptors;

public:
  BindDescriptorSetsCommand(VkPipelineBindPoint const &bindPoint, VkPipelineLayout const &layout,
                            std::vector<VkDescriptorSet> const &descriptors)
      : bindPoint(bindPoint), layout(layout), descriptors(descriptors) {}
  BindDescriptorSetsCommand(VkPipelineBindPoint const &bindPoint, VkPipelineLayout const &layout,
                            VkDescriptorSet const &descriptor)
      : bindPoint(bindPoint), layout(layout), descriptors{descriptor} {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    vkCmdBindDescriptorSets(queue, bindPoint, layout, 0, static_cast<uint32_t>(descriptors.size()), descriptors.data(),
                            0, nullptr);
  }
};

class DispatchCommand : public Command {
  uint32_t gx, gy, gz;

public:
  DispatchCommand(uint32_t workerGroupsX, uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : gx(workerGroupsX), gy(workerGroupsY), gz(workerGroupsZ) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const { vkCmdDispatch(queue, gx, gy, gz); }
};

template <typename T> class PushConstantsCommand : public Command {
  T constants;
  VkPipelineLayout pipelineLayout;

public:
  PushConstantsCommand(T const &constants, VkPipelineLayout const &pipelineLayout)
      : constants(constants), pipelineLayout(pipelineLayout) {}
  void QueueExecution(VkCommandBuffer const &queue) const {
    vkCmdPushConstants(queue, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(T), &constants);
  }
};

template <typename T> class ExecuteComputePipelineCommand : public Command {
  BindPipelineCommand bindPipeline;
  BindDescriptorSetsCommand bindDescriptors;
  DispatchCommand dispatch;
  PushConstantsCommand<T> pushConstants;

public:
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, std::vector<VkDescriptorSet> const &descriptors,
                                T const &pushConstants, uint32_t workerGroupsX, uint32_t workerGroupsY,
                                uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptors),
        dispatch(workerGroupsX, workerGroupsY, workerGroupsZ), pushConstants(pushConstants, layout) {}
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, VkDescriptorSet const &descriptor,
                                T const &pushConstants, uint32_t workerGroupsX, uint32_t workerGroupsY,
                                uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptor),
        pushConstants(pushConstants, layout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  ExecuteComputePipelineCommand(ComputeEffect<T> const &effect, VkPipelineBindPoint const &bindPoint,
                                VkDescriptorSet const &descriptor, uint32_t workerGroupsX, uint32_t workerGroupsY,
                                uint32_t workerGroupsZ)
      : bindPipeline(effect.pipeline, bindPoint), bindDescriptors(bindPoint, effect.pipelineLayout, descriptor),
        pushConstants(effect.constants, effect.pipelineLayout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    bindPipeline.QueueExecution(queue);
    bindDescriptors.QueueExecution(queue);
    pushConstants.QueueExecution(queue);
    dispatch.QueueExecution(queue);
  }
};

} // namespace Engine::Graphics
