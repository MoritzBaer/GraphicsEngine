#include "CommandQueue.h"
#include "InstanceManager.h"
#include "../Macros.h"
#include "../Debug/Logging.h"
#include "VulkanUtil.h"

void Engine::Graphics::CommandQueue::Create()
{
    VkCommandPoolCreateInfo commandPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = InstanceManager::GetGraphicsFamily(),
    };
    
    InstanceManager::CreateCommandPool(&commandPoolInfo, &commandPool);
    
    VkCommandBufferAllocateInfo commandBufferInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    InstanceManager::AllocateCommandBuffers(&commandBufferInfo, &mainBuffer);
}

void Engine::Graphics::CommandQueue::Destroy()
{
    InstanceManager::FreeCommandBuffers(commandPool, &mainBuffer);
    InstanceManager::DestroyCommandPool(commandPool);
}

VkCommandBufferSubmitInfo Engine::Graphics::CommandQueue::EnqueueCommandSequence(std::initializer_list<Command const *> commands, VkCommandBufferUsageFlags flags) const
{
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = flags
    };

    VULKAN_ASSERT(vkResetCommandBuffer(mainBuffer, 0), "Failed to reset command buffer!")

    VULKAN_ASSERT(vkBeginCommandBuffer(mainBuffer, &beginInfo), "Failed to begin command buffer!")

    for(auto command : commands) { command->QueueExecution(mainBuffer); }

    VULKAN_ASSERT(vkEndCommandBuffer(mainBuffer), "Failed to end command buffer!")

    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = mainBuffer,
        .deviceMask = 0
    };
}

Engine::Graphics::PipelineBarrierCommand::PipelineBarrierCommand(std::vector<VkImageMemoryBarrier2> const & imageMemoryBarriers) : imageMemoryBarriers(imageMemoryBarriers) { }

void Engine::Graphics::PipelineBarrierCommand::QueueExecution(VkCommandBuffer const &queue) const
{
    auto dependencies = vkinit::DependencyInfo(imageMemoryBarriers);
    vkCmdPipelineBarrier2(queue, &dependencies);
}

Engine::Graphics::ClearColourCommand::ClearColourCommand(
    VkImage image, 
    VkImageLayout currentLayout, 
    VkClearColorValue const & clearValue, 
    std::vector<VkImageSubresourceRange> const & subresourceRanges) :

    image(image), 
    currentLayout(currentLayout), 
    clearColour(clearValue), 
    subresourceRanges(subresourceRanges) 

    {}

void Engine::Graphics::ClearColourCommand::QueueExecution(VkCommandBuffer const &queue) const
{
    vkCmdClearColorImage(queue, image, currentLayout, &clearColour, static_cast<uint32_t>(subresourceRanges.size()), subresourceRanges.data());
}
