#include "DebugGUIRenderingStrategy.h"

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

std::vector<Engine::Graphics::Command *> Editor::DebugGUIRenderingStrategy::GetRenderingCommands(
    Engine::Graphics::RenderingRequest const &request, Engine::Graphics::Image<2> &colourImage,
    Engine::Graphics::Image<2> &depthImage, Engine::Maths::Dimension2 const &renderDimension,
    Engine::Graphics::Buffer<Engine::Graphics::DrawData> const &uniformBuffer,
    Engine::Graphics::DescriptorAllocator &descriptorAllocator, Engine::Graphics::DescriptorWriter &descriptorWriter,
    Engine::Graphics::Image<2> &swapchainImage) {

  auto renderingCommands =
      renderingStrategy->GetRenderingCommands(request, colourImage, depthImage, renderDimension, uniformBuffer,
                                              descriptorAllocator, descriptorWriter, swapchainImage);

  // Commands for rendering ImGUI
  auto transitionPresenterToColourAttachment = swapchainImage.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  auto renderImGUI = new ImGUIDrawCommand(swapchainImage);

  renderingCommands.push_back(transitionPresenterToColourAttachment);
  renderingCommands.push_back(renderImGUI);

  return renderingCommands;
}