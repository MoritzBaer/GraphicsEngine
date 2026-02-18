#pragma once

#include "vulkan/vulkan.h"

namespace Engine::Graphics {
class Command {
public:
  virtual void QueueExecution(VkCommandBuffer const &queue) const = 0;
};
} // namespace Engine::Graphics