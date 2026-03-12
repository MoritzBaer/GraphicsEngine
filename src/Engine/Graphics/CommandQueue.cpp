#include "CommandQueue.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "InstanceManager.h"
#include "Util/Macros.h"
#include "VulkanUtil.h"
#include <cstddef>

Engine::Graphics::CommandRecorder Engine::Graphics::CommandQueue::GetRecorder(VkCommandBufferUsageFlags flags) const {
  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = flags};

  VULKAN_ASSERT(vkResetCommandBuffer(mainBuffer, 0), "Failed to reset command buffer!")

  VULKAN_ASSERT(vkBeginCommandBuffer(mainBuffer, &beginInfo), "Failed to begin command buffer!")

  return CommandRecorder{mainBuffer};
}

VkCommandBufferSubmitInfo
Engine::Graphics::CommandQueue::EnqueueCommandRecord(Engine::Graphics::CommandRecorder const &commands) const {
  PROFILE_FUNCTION()

  VULKAN_ASSERT(vkEndCommandBuffer(mainBuffer), "Failed to end command buffer!")

  return {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = mainBuffer, .deviceMask = 0};
}