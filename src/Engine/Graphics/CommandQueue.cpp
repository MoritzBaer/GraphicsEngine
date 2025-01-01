#include "CommandQueue.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "InstanceManager.h"
#include "Util/Macros.h"
#include "VulkanUtil.h"

VkCommandBufferSubmitInfo
Engine::Graphics::CommandQueue::EnqueueCommandSequence(std::span<Command const *> const &commands,
                                                       VkCommandBufferUsageFlags flags) const {
  PROFILE_FUNCTION()
  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = flags};

  VULKAN_ASSERT(vkResetCommandBuffer(mainBuffer, 0), "Failed to reset command buffer!")

  VULKAN_ASSERT(vkBeginCommandBuffer(mainBuffer, &beginInfo), "Failed to begin command buffer!")

  for (auto command : commands) {
    PROFILE_SCOPE("Queueing command") command->QueueExecution(mainBuffer);
    delete command;
  }

  VULKAN_ASSERT(vkEndCommandBuffer(mainBuffer), "Failed to end command buffer!")

  return {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = mainBuffer, .deviceMask = 0};
}