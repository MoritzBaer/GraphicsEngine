#pragma once

#include "CommandQueue.h"
#include "InstanceManager.h"
#include "VulkanUtil.h"

namespace Engine::Graphics {
class GPUDispatcher {
  InstanceManager const *instanceManager;

  VkFence fence;
  CommandQueue commandQueue;
  VkQueue dispatchQueue;

public:
  GPUDispatcher(InstanceManager const *instanceManager, CommandQueue const &commandQueue)
      : instanceManager(instanceManager), commandQueue(commandQueue) {
    auto fenceInfo = vkinit::FenceCreateInfo();
    instanceManager->CreateFence(&fenceInfo, &fence);
    instanceManager->GetGraphicsQueue(&dispatchQueue);
  }

  ~GPUDispatcher() { instanceManager->DestroyFence(fence); }

  void Dispatch(std::span<Command const *> const &commands) const;

  inline void Dispatch(Command const *command) const {
    std::vector<Command const *> commandSpan = {command};
    Dispatch(commandSpan);
  }
};
} // namespace Engine::Graphics
