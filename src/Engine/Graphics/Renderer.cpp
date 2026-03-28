#include "Renderer.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Graphics/CommandQueue.h"
#include "UniformAggregate.h"
#include "VulkanUtil.h"
#include <algorithm>
#include <vector>

namespace Engine::Graphics {

Renderer::Renderer(InstanceManager const *instanceManager) : instanceManager(instanceManager) {
  PROFILE_FUNCTION()
  instanceManager->GetGraphicsQueue(&graphicsQueue);
}

Renderer::~Renderer() {}

void Renderer::DrawFrame(RenderingRequest const &request) {

  PROFILE_FUNCTION()

  VkCommandBufferSubmitInfo commandBufferSubmitInfo{};

  auto frameResources = renderResourceProvider->GetFrameResources();

  {
    PROFILE_SCOPE("Waiting for previous frame to finish rendering")
    instanceManager->WaitForFences(&frameResources.renderFence);
    instanceManager->ResetFences(&frameResources.renderFence);
  }

  CommandRecorder commands = frameResources.commandQueue.GetRecorder();

  auto renderTargetOption = renderResourceProvider->GetRenderTarget(commands);

  if (!renderTargetOption) {
    return;
  }

  auto renderTarget = renderTargetOption.value();

  {
    PROFILE_SCOPE("Generate commands")

    renderResourceProvider->PrepareTargetForRendering(commands);
    renderingStrategy->RecordRenderingCommands(request, frameResources.uniformBinder,
                                               frameResources.descriptorAllocator, frameResources.descriptorWriter,
                                               renderTarget.renderBuffer, commands);

    renderResourceProvider->PrepareTargetForDisplaying(commands);

    commandBufferSubmitInfo = frameResources.commandQueue.EnqueueCommandRecord(commands);
  }

  std::vector<VkSemaphoreSubmitInfo> semaphoreWaitInfo{};
  if (frameResources.renderSemaphore) {
    semaphoreWaitInfo.push_back(vkinit::SemaphoreSubmitInfo(frameResources.renderSemaphore.value(),
                                                            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR));
  }
  std::vector<VkSemaphoreSubmitInfo> semaphoreSignalInfo{};
  if (renderTarget.presentSemaphore) {
    semaphoreSignalInfo.push_back(
        vkinit::SemaphoreSubmitInfo(renderTarget.presentSemaphore.value(), VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT));
  }

  std::vector<VkCommandBufferSubmitInfo> commandBuffers = {commandBufferSubmitInfo};

  VkSubmitInfo2 submitInfo = vkinit::SubmitInfo(semaphoreWaitInfo, commandBuffers, semaphoreSignalInfo);

  VULKAN_ASSERT(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frameResources.renderFence), "Failed to submit queue")

  renderResourceProvider->DisplayRenderTarget(commands);
}

} // namespace Engine::Graphics