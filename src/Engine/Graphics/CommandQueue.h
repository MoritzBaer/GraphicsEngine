#pragma once

#include "Util/DeletionQueue.h"
#include "vulkan/vulkan.h"
#include <span>

namespace Engine::Graphics {

class GPUObjectManager;

class Command {
public:
  virtual void QueueExecution(VkCommandBuffer const &queue) const = 0;
};

class CommandQueue {
  friend class GPUObjectManager;

private:
  VkCommandPool commandPool;
  VkCommandBuffer mainBuffer;

public:
  CommandQueue(VkCommandPool commandPool, VkCommandBuffer mainBuffer)
      : commandPool(commandPool), mainBuffer(mainBuffer) {}
  CommandQueue() : CommandQueue(VK_NULL_HANDLE, VK_NULL_HANDLE) {}

  VkCommandBufferSubmitInfo EnqueueCommandSequence(std::span<Command const *> const &commands,
                                                   VkCommandBufferUsageFlags flags = 0) const;
};

class CompositeCommand : public Command {
  std::span<Command const *> commands;

public:
  CompositeCommand(std::span<Command const *> const &commands) : commands(commands) {}
  void QueueExecution(VkCommandBuffer const &queue) const {
    for (Command const *command : commands) {
      command->QueueExecution(queue);
      delete command;
    }
  }
};

} // namespace Engine::Graphics
