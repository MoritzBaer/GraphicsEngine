#pragma once

#include "Maths/Dimension.h"
#include "vulkan/vulkan.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics::vkinit {
inline VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask) {
  return {.aspectMask = aspectMask,
          .baseMipLevel = 0,
          .levelCount = VK_REMAINING_MIP_LEVELS,
          .baseArrayLayer = 0,
          .layerCount = VK_REMAINING_ARRAY_LAYERS};
}

inline VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask) {
  return {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
          .semaphore = semaphore,
          .value = 1,
          .stageMask = stageMask,
          .deviceIndex = 0};
}

inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT) {
  return {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags};
}

inline VkImageMemoryBarrier2 ImageMemoryBarrier(VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout,
                                                VkImageAspectFlags aspect) {
  return {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
          .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
          .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
          .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
          .oldLayout = currentLayout,
          .newLayout = targetLayout,
          .image = image,
          .subresourceRange = ImageSubresourceRange(aspect)};
}

inline VkDependencyInfo DependencyInfo(std::vector<VkImageMemoryBarrier2> const &imageMemoryBarriers) {
  return {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
          .imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size()),
          .pImageMemoryBarriers = imageMemoryBarriers.data()};
}

inline VkRenderingInfo RenderingInfo(VkRenderingAttachmentInfo const &colourAttachmentInfo, VkExtent2D extent) {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
          .renderArea{.offset = {0, 0}, .extent = extent},
          .layerCount = 1,
          .colorAttachmentCount = 1,
          .pColorAttachments = &colourAttachmentInfo};
}

inline VkRenderingInfo RenderingInfo(VkRenderingAttachmentInfo const &colourAttachmentInfo,
                                     VkRenderingAttachmentInfo const &depthAttachmentInfo, VkExtent2D extent) {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
          .renderArea{.offset = {0, 0}, .extent = extent},
          .layerCount = 1,
          .colorAttachmentCount = 1,
          .pColorAttachments = &colourAttachmentInfo,
          .pDepthAttachment = &depthAttachmentInfo};
}

inline VkRenderingAttachmentInfo ColourAttachmentInfo(VkImageView const &imageView,
                                                      VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                                                      VkClearColorValue const &clearColour = {0, 0, 0, 0}) {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .imageView = imageView,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = loadOp,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue{.color = clearColour}};
}

inline VkRenderingAttachmentInfo DepthAttachmentInfo(VkImageView const &imageView,
                                                     VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                     VkClearDepthStencilValue const &clearValue = {.depth = 1.0f}) {
  return {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
          .imageView = imageView,
          .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
          .loadOp = loadOp,
          .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
          .clearValue{.depthStencil = clearValue}};
}

inline VkSubmitInfo2 SubmitInfo(std::vector<VkSemaphoreSubmitInfo> const &semaphoreWaitInfos,
                                std::vector<VkCommandBufferSubmitInfo> const &commandBufferInfos,
                                std::vector<VkSemaphoreSubmitInfo> const &semaphoreSignalInfos) {
  return {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
          .waitSemaphoreInfoCount = static_cast<uint32_t>(semaphoreWaitInfos.size()),
          .pWaitSemaphoreInfos = semaphoreWaitInfos.data(),
          .commandBufferInfoCount = static_cast<uint32_t>(commandBufferInfos.size()),
          .pCommandBufferInfos = commandBufferInfos.data(),
          .signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignalInfos.size()),
          .pSignalSemaphoreInfos = semaphoreSignalInfos.data()};
}

inline VkSubmitInfo2 SubmitInfo(VkSemaphoreSubmitInfo const &semaphoreWaitInfo,
                                VkCommandBufferSubmitInfo const &commandBufferInfo,
                                VkSemaphoreSubmitInfo const &semaphoreSignalInfo) {
  return {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
          .waitSemaphoreInfoCount = 1,
          .pWaitSemaphoreInfos = &semaphoreWaitInfo,
          .commandBufferInfoCount = 1,
          .pCommandBufferInfos = &commandBufferInfo,
          .signalSemaphoreInfoCount = 1,
          .pSignalSemaphoreInfos = &semaphoreSignalInfo};
}

} // namespace Engine::Graphics::vkinit

namespace Engine::Graphics::vkutil {

template <uint8_t D> inline VkExtent3D DimensionToExtent(Maths::Dimension<D> dimension);
template <uint8_t D> inline VkOffset3D DimensionToOffset(Maths::Dimension<D> dimension);

template <> inline VkExtent3D DimensionToExtent(Maths::Dimension<1> dimension) {
  return VkExtent3D{dimension.x(), 1, 1};
}
template <> inline VkExtent3D DimensionToExtent(Maths::Dimension<2> dimension) {
  return VkExtent3D{dimension.x(), dimension.y(), 1};
}
template <> inline VkExtent3D DimensionToExtent(Maths::Dimension<3> dimension) {
  return VkExtent3D{dimension.x(), dimension.y(), dimension.z()};
}

template <> inline VkOffset3D DimensionToOffset(Maths::Dimension<1> dimension) {
  return VkOffset3D{static_cast<int32_t>(dimension.x()), 0, 0};
}
template <> inline VkOffset3D DimensionToOffset(Maths::Dimension<2> dimension) {
  return VkOffset3D{static_cast<int32_t>(dimension.x()), static_cast<int32_t>(dimension.y()), 0};
}
template <> inline VkOffset3D DimensionToOffset(Maths::Dimension<3> dimension) {
  return VkOffset3D{static_cast<int32_t>(dimension.x()), static_cast<int32_t>(dimension.y()),
                    static_cast<int32_t>(dimension.z())};
}

inline VkViewport MakeViewport(Maths::Dimension2 const &dimension,
                               Maths::Dimension2 const &offset = Maths::Dimension2::Zero, float minDepth = 0,
                               float maxDepth = 1) {
  return VkViewport{.x = static_cast<float>(offset.x()),
                    .y = static_cast<float>(offset.y()),
                    .width = static_cast<float>(dimension.x()),
                    .height = static_cast<float>(dimension.y()),
                    .minDepth = minDepth,
                    .maxDepth = maxDepth};
}

inline VkRect2D MakeRect(Maths::Dimension2 const &dimension,
                         Maths::Dimension2 const &offset = Maths::Dimension2::Zero) {
  return {.offset = {.x = static_cast<int32_t>(offset.x()), .y = static_cast<int32_t>(offset.y())},
          .extent = {.width = dimension.x(), .height = dimension.y()}};
}

} // namespace Engine::Graphics::vkutil
