#pragma once

#include "CommandQueue.h"
#include "Debug/Logging.h"
#include "InstanceManager.h"
#include "VulkanUtil.h"

namespace Engine::Graphics {
class GPUDispatcher {
  InstanceManager &instanceManager;

  VkFence fence;
  CommandQueue commandQueue;
  VkQueue dispatchQueue;

public:
  GPUDispatcher(InstanceManager &instanceManager, CommandQueue const &)
      : instanceManager(instanceManager), commandQueue(commandQueue) {
    auto fenceInfo = vkinit::FenceCreateInfo();
    instanceManager.CreateFence(&fenceInfo, &fence);
    instanceManager.GetGraphicsQueue(&dispatchQueue);
  }

  ~GPUDispatcher() { instanceManager.DestroyFence(fence); }

  inline void Dispatch(Command *command) const {
    instanceManager.ResetFences(&fence);
    auto commandInfo = commandQueue.EnqueueCommandSequence({command});
    std::vector<VkCommandBufferSubmitInfo> buffer{commandInfo};
    VkSubmitInfo2 submitInfo = vkinit::SubmitInfo({}, buffer, {});

    VULKAN_ASSERT(vkQueueSubmit2(dispatchQueue, 1, &submitInfo, fence), "Failed to submit immediate queue")

    instanceManager.WaitForFences(&fence);
  }
};
} // namespace Engine::Graphics
