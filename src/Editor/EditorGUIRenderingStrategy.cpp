#include "Editor/EditorGUIRenderingStrategy.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/VulkanUtil.h"

class ImGUIDrawCommand : public Engine::Graphics::Command {
  Engine::Graphics::Image2 const &targetImage;

public:
  ImGUIDrawCommand(Engine::Graphics::Image2 const &targetImage) : targetImage(targetImage) {}
  void QueueExecution(VkCommandBuffer const &queue) const;
};

void ImGUIDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  if (auto drawData = ImGui::GetDrawData()) {
    auto colourInfo = targetImage.BindAsColourAttachment();
    VkExtent2D extent = {targetImage.GetExtent().x(), targetImage.GetExtent().y()};
    VkRenderingInfo renderInfo = Engine::Graphics::vkinit::RenderingInfo(colourInfo, extent);

    vkCmdBeginRendering(queue, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), queue);

    vkCmdEndRendering(queue);
  }
}

std::vector<Engine::Graphics::Command *> Editor::EditorGUIRenderingStrategy::GetRenderingCommands(
    Engine::Graphics::RenderingRequest const &request, Engine::Maths::Dimension2 const &renderDimension,
    Engine::Graphics::Buffer<Engine::Graphics::DrawData> const &uniformBuffer,
    Engine::Graphics::DescriptorAllocator &descriptorAllocator, Engine::Graphics::DescriptorWriter &descriptorWriter,
    Engine::Graphics::Image<2> &renderTarget) {

  auto renderingCommands = subStrategy->GetRenderingCommands(request, targetResolution, uniformBuffer,
                                                             descriptorAllocator, descriptorWriter, renderBuffer);

  auto transitionBufferToTransferSrc = renderBuffer.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  auto transitionTargetToTransferDst = renderTarget.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  auto copyBufferToTarget = renderBuffer.BlitTo(renderTarget);

  renderingCommands.push_back(transitionBufferToTransferSrc);
  renderingCommands.push_back(transitionTargetToTransferDst);
  renderingCommands.push_back(copyBufferToTarget);

  // Commands for rendering ImGUI
  auto transitionPresenterToColourAttachment = renderTarget.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  auto renderImGUI = new ImGUIDrawCommand(renderTarget);

  renderingCommands.push_back(transitionPresenterToColourAttachment);
  renderingCommands.push_back(renderImGUI);

  return renderingCommands;
}

void Editor::EditorGUIRenderingStrategy::CreateRenderBuffer() {
  renderBuffer =
      objectManager->CreateAllocatedImage(VK_FORMAT_R8G8B8A8_UNORM, targetResolution,
                                          VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                          VK_IMAGE_ASPECT_COLOR_BIT);
}
void Editor::EditorGUIRenderingStrategy::DestroyRenderBuffer() { objectManager->DestroyAllocatedImage(renderBuffer); }