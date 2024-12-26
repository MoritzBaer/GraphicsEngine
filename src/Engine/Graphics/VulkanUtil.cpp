#include "VulkanUtil.h"

Engine::Graphics::vkutil::PipelineBarrierCommand::PipelineBarrierCommand(
    std::vector<VkImageMemoryBarrier2> const &imageMemoryBarriers)
    : imageMemoryBarriers(imageMemoryBarriers) {}

void Engine::Graphics::vkutil::PipelineBarrierCommand::QueueExecution(VkCommandBuffer const &queue) const {
  auto dependencies = vkinit::DependencyInfo(imageMemoryBarriers);
  vkCmdPipelineBarrier2(queue, &dependencies);
}

Engine::Graphics::vkutil::ClearColourCommand::ClearColourCommand(
    VkImage image, VkImageLayout currentLayout, VkClearColorValue const &clearValue,
    std::vector<VkImageSubresourceRange> const &subresourceRanges)
    :

      image(image), currentLayout(currentLayout), clearColour(clearValue), subresourceRanges(subresourceRanges)

{}

void Engine::Graphics::vkutil::ClearColourCommand::QueueExecution(VkCommandBuffer const &queue) const {
  vkCmdClearColorImage(queue, image, currentLayout, &clearColour, static_cast<uint32_t>(subresourceRanges.size()),
                       subresourceRanges.data());
}

void Engine::Graphics::vkutil::BlitImageCommand::QueueExecution(VkCommandBuffer const &queue) const {
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
