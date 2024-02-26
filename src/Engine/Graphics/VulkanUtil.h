#pragma once

#include "vulkan/vulkan.h"
#include <vector>

namespace Engine::Graphics::vkinit
{
    inline VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask) {
        return {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS
            };
    }

    inline VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkSemaphore semaphore, VkPipelineStageFlags2 stageMask)
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore,
            .value = 1,
            .stageMask = stageMask,
            .deviceIndex = 0
        };
    }

    inline VkImageMemoryBarrier2 ImageMemoryBarrier(VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout) {
        return {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
            .oldLayout = currentLayout,
            .newLayout = targetLayout,
            .image = image,
            .subresourceRange = ImageSubresourceRange((currentLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT)
        };
    }

    inline VkDependencyInfo DependencyInfo(std::vector<VkImageMemoryBarrier2> const & imageMemoryBarriers) {
        return {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size()),
            .pImageMemoryBarriers = imageMemoryBarriers.data()
        };
    }
} // namespace Engine::Graphics::vkinit

namespace Engine::Graphics::vkutil
{
    
} // namespace Engine::Graphics::vkutil
