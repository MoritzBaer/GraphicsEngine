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

  auto frameResources = renderResourceProvider->GetFrameResources();

  {
    PROFILE_SCOPE("Waiting for previous frame to finish rendering")
    instanceManager->WaitForFences(&frameResources.renderFence);
    instanceManager->ResetFences(&frameResources.renderFence);
  }

  bool acquisitionSuccessful = false;
  auto &renderTarget = renderResourceProvider->GetRenderTarget(acquisitionSuccessful);

  if (!acquisitionSuccessful) {
    return;
  }

  {
    PROFILE_SCOPE("Generate commands")

    std::vector<Command const *> commands;

    auto prepareTarget = renderResourceProvider->PrepareTargetForRendering();

    commands.insert(commands.end(), prepareTarget.begin(), prepareTarget.end());

    DescriptorWriter descriptorWriter(instanceManager);
    auto strategyCommands = renderingStrategy->GetRenderingCommands(
        request, frameResources.uniformBuffer, frameResources.descriptorAllocator, descriptorWriter, renderTarget);
    commands.insert(commands.end(), strategyCommands.begin(), strategyCommands.end());

    prepareTarget = renderResourceProvider->PrepareTargetForDisplaying();

    commands.insert(commands.end(), prepareTarget.begin(), prepareTarget.end());

    commandBufferSubmitInfo = frameResources.commandQueue.EnqueueCommandSequence(commands);
  }

  std::vector<VkSemaphoreSubmitInfo> semaphoreWaitInfo{};
  if (frameResources.presentSemaphore) {
    semaphoreWaitInfo.push_back(vkinit::SemaphoreSubmitInfo(frameResources.presentSemaphore,
                                                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR));
  }
  std::vector<VkSemaphoreSubmitInfo> semaphoreSignalInfo{};
  if (frameResources.renderSemaphore) {
    semaphoreSignalInfo.push_back(
        vkinit::SemaphoreSubmitInfo(frameResources.renderSemaphore, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT));
  }

  std::vector<VkCommandBufferSubmitInfo> commandBuffers = {commandBufferSubmitInfo};

  VkSubmitInfo2 submitInfo = vkinit::SubmitInfo(semaphoreWaitInfo, commandBuffers, semaphoreSignalInfo);

  VULKAN_ASSERT(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameResources.renderFence), "Failed to submit queue")

  renderResourceProvider->DisplayRenderTarget();
}

} // namespace Engine::Graphics