#include "GPUDispatcher.h"

#include "Debug/Logging.h"

void Engine::Graphics::GPUDispatcher::Dispatch(std::span<Command const *> const &commands) const {
  instanceManager->ResetFences(&fence);
  auto commandInfo = commandQueue.EnqueueCommandSequence(commands);
  std::vector<VkCommandBufferSubmitInfo> buffer{commandInfo};
  VkSubmitInfo2 submitInfo = vkinit::SubmitInfo({}, buffer, {});

  VULKAN_ASSERT(vkQueueSubmit2(dispatchQueue, 1, &submitInfo, fence), "Failed to submit immediate queue")

  instanceManager->WaitForFences(&fence);
}