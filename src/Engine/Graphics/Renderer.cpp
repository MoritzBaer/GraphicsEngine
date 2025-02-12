#include "Renderer.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "UniformAggregate.h"
#include "VulkanUtil.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {

Renderer::Renderer(InstanceManager const *instanceManager, GPUObjectManager const *gpuObjectManager)
    : instanceManager(instanceManager), descriptorLayoutBuilder(instanceManager), descriptorWriter(instanceManager),
      gpuObjectManager(gpuObjectManager) {
  PROFILE_FUNCTION()
  instanceManager->GetGraphicsQueue(&graphicsQueue);
}

Renderer::~Renderer() {
  descriptorAllocator.ClearDescriptors();
  descriptorAllocator.DestroyPools();
}

void Renderer::DrawFrame(RenderingRequest const &request) {

  PROFILE_FUNCTION()

  VkCommandBufferSubmitInfo commandBufferSubmitInfo{};

  auto frameResources = frameResourceProvider->GetRenderResources();

  if (!frameResources.renderTarget) {
    return;
  }

  {
    PROFILE_SCOPE("Generate commands")

    std::vector<Command const *> commands;

    auto prepareTarget = frameResourceProvider->PrepareTargetForRendering();

    commands.insert(commands.end(), prepareTarget.begin(), prepareTarget.end());

    DescriptorWriter descriptorWriter(instanceManager);
    auto strategyCommands = renderingStrategy->GetRenderingCommands(
        request, windowDimension, frameResources.uniformBuffer, frameResources.descriptorAllocator, descriptorWriter,
        *frameResources.renderTarget);
    commands.insert(commands.end(), strategyCommands.begin(), strategyCommands.end());

    prepareTarget = frameResourceProvider->PrepareTargetForDisplaying();

    commands.insert(commands.end(), prepareTarget.begin(), prepareTarget.end());

    commandBufferSubmitInfo = frameResources.commandQueue.EnqueueCommandSequence(commands);
  }

  auto semaphoreWaitInfo =
      vkinit::SemaphoreSubmitInfo(frameResources.presentSemaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR);
  auto semaphoreSignalInfo =
      vkinit::SemaphoreSubmitInfo(frameResources.renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);

  VkSubmitInfo2 submitInfo = vkinit::SubmitInfo(semaphoreWaitInfo, commandBufferSubmitInfo, semaphoreSignalInfo);

  VULKAN_ASSERT(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameResources.renderFence), "Failed to submit queue")

  frameResourceProvider->DisplayRenderTarget();
}

} // namespace Engine::Graphics