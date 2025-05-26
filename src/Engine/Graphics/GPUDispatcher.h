#pragma once

#include "CommandQueue.h"
#include "InstanceManager.h"
#include "VulkanUtil.h"

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

  void Dispatch(std::span<Command const *> const &commands) const;

  inline void Dispatch(Command const *command) const {
    std::vector<Command const *> commandSpan = {command};
    Dispatch(commandSpan);
  }
};
} // namespace Engine::Graphics
