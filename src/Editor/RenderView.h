#pragma once

#include "Graphics/RenderTargetProvider.h"
#include "ImGUIManager.h"

using namespace Engine::Graphics;

namespace Editor {

struct RenderView : public Engine::Graphics::ImGUIView, public Engine::Graphics::RenderResourceProvider {
  Engine::Graphics::RenderResourceProvider::FrameResources frameResources;

  Engine::Graphics::Texture2D renderTarget;
  VkDescriptorSet textureDescriptor;

  bool renderTextureInitialized = false;

  RenderView(Engine::Graphics::GPUObjectManager
#ifdef NDEBUG
             const
#endif
                 *objectManager,
             Engine::Graphics::InstanceManager const *instanceManager)
      : Engine::Graphics::ImGUIView("Game"), Engine::Graphics::RenderResourceProvider(),
        renderTarget(objectManager->CreateTexture(Maths::Dimension2{1600, 900}, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
                                                  VK_FORMAT_R8G8B8A8_UNORM, true, VK_SAMPLE_COUNT_1_BIT,
                                                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)) {

    VkFenceCreateInfo fenceInfo = vkinit::FenceCreateInfo();

    VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    instanceManager->CreateFence(&fenceInfo, &frameResources.renderFence);
    frameResources.renderSemaphore = VK_NULL_HANDLE;
    frameResources.presentSemaphore = VK_NULL_HANDLE;

    frameResources.commandQueue = objectManager->CreateCommandQueue();

    std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    };

    frameResources.descriptorWriter = DescriptorWriter(instanceManager);
    frameResources.descriptorAllocator = DescriptorAllocator(instanceManager);
    frameResources.descriptorAllocator.InitPools(10, frame_sizes);
    frameResources.uniformBuffer =
        objectManager->CreateBuffer<DrawData>(1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  }

  FrameResources GetFrameResources() override { return frameResources; }

  Engine::Graphics::Image2 &GetRenderTarget(bool &acquisitionSuccessful) override {
    acquisitionSuccessful = true;
    return renderTarget;
  }

  std::vector<Engine::Graphics::Command const *> PrepareTargetForRendering() override {
    return {renderTarget.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)};
  }

  std::vector<Engine::Graphics::Command const *> PrepareTargetForDisplaying() override {
    return {renderTarget.Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)};
  }

  void DisplayRenderTarget() override {
    if (!renderTextureInitialized) {
      textureDescriptor = renderTarget.AddToImGui();
      renderTextureInitialized = true;
    }
  }

  void DrawContent() override {
    if (renderTextureInitialized) {
      ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
      auto textureDimension = renderTarget.GetExtent();
      float textureAspect = static_cast<float>(textureDimension.x()) / textureDimension.y();
      float viewportAspect = viewportPanelSize.x / viewportPanelSize.y;
      ImVec2 panelSize = {textureAspect > viewportAspect ? viewportPanelSize.x : viewportPanelSize.y * textureAspect,
                          textureAspect > viewportAspect ? viewportPanelSize.x / textureAspect : viewportPanelSize.y};
      ImGui::Image(textureDescriptor, ImVec2{panelSize.x, panelSize.y});
    }
  }
};

} // namespace Editor