#pragma once

#include "CommandQueue.h"
#include "InstanceManager.h"
#include "VulkanUtil.h"
#include <type_traits>

namespace Engine::Graphics {

class GPUObjectManager;
class GPUDispatcher {
  InstanceManager const *instanceManager;

  VkFence fence;
  CommandQueue commandQueue;
  VkQueue dispatchQueue;

  friend class GPUObjectManager;

public:
  GPUDispatcher(InstanceManager const *instanceManager = nullptr, CommandQueue const &commandQueue = CommandQueue())
      : instanceManager(instanceManager), commandQueue(commandQueue) {
    auto fenceInfo = vkinit::FenceCreateInfo();
    if (instanceManager) {
      instanceManager->CreateFence(&fenceInfo, &fence);
      instanceManager->GetGraphicsQueue(&dispatchQueue);
    }
  }

  void Dispatch(CommandSet auto const &commands) const {
    instanceManager->ResetFences(&fence);
    auto commandInfo = commandQueue.EnqueueCommands(commands);
    std::vector<VkCommandBufferSubmitInfo> buffer{commandInfo};
    VkSubmitInfo2 submitInfo = vkinit::SubmitInfo({}, buffer, {});

    VULKAN_ASSERT(vkQueueSubmit2(dispatchQueue, 1, &submitInfo, fence), "Failed to submit immediate queue")

    instanceManager->WaitForFences(&fence);
  }
};
} // namespace Engine::Graphics
