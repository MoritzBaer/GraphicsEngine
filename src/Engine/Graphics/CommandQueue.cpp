#include "CommandQueue.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "InstanceManager.h"
#include "Util/Macros.h"
#include "VulkanUtil.h"

VkCommandBufferSubmitInfo
Engine::Graphics::CommandQueue::EnqueueCommandSequence(std::initializer_list<Command const *> commands,
                                                       VkCommandBufferUsageFlags flags) const {
  PROFILE_FUNCTION()
  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = flags};

  VULKAN_ASSERT(vkResetCommandBuffer(mainBuffer, 0), "Failed to reset command buffer!")

  VULKAN_ASSERT(vkBeginCommandBuffer(mainBuffer, &beginInfo), "Failed to begin command buffer!")

  for (auto command : commands) {
    PROFILE_SCOPE("Queueing command") command->QueueExecution(mainBuffer);
  }

  VULKAN_ASSERT(vkEndCommandBuffer(mainBuffer), "Failed to end command buffer!")

  return {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = mainBuffer, .deviceMask = 0};
}

Engine::Graphics::PipelineBarrierCommand::PipelineBarrierCommand(
    std::vector<VkImageMemoryBarrier2> const &imageMemoryBarriers)
    : imageMemoryBarriers(imageMemoryBarriers) {}

void Engine::Graphics::PipelineBarrierCommand::QueueExecution(VkCommandBuffer const &queue) const {
  auto dependencies = vkinit::DependencyInfo(imageMemoryBarriers);
  vkCmdPipelineBarrier2(queue, &dependencies);
}

Engine::Graphics::ClearColourCommand::ClearColourCommand(VkImage image, VkImageLayout currentLayout,
                                                         VkClearColorValue const &clearValue,
                                                         std::vector<VkImageSubresourceRange> const &subresourceRanges)
    :

      image(image), currentLayout(currentLayout), clearColour(clearValue), subresourceRanges(subresourceRanges)

{}

void Engine::Graphics::ClearColourCommand::QueueExecution(VkCommandBuffer const &queue) const {
  vkCmdClearColorImage(queue, image, currentLayout, &clearColour, static_cast<uint32_t>(subresourceRanges.size()),
                       subresourceRanges.data());
}

void Engine::Graphics::BlitImageCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkBlitImageInfo2 blitInfo = {
      .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
      .srcImage = source,
      .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      .dstImage = destination,
      .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      .regionCount = static_cast<uint32_t>(blitRegions.size()),
      .pRegions = blitRegions.data(),
      .filter = VK_FILTER_LINEAR,
  };

  vkCmdBlitImage2(queue, &blitInfo);
}
