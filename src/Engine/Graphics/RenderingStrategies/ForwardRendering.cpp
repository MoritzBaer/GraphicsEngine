#include "ForwardRendering.h"

namespace Engine::Graphics::RenderingStrategies {

class MultimeshDrawCommand : public Command {
  Image<2> const &drawImage;
  Image<2> const &depthImage;
  Maths::Dimension2 renderAreaSize;
  Maths::Dimension2 renderAreaOffset;
  DescriptorAllocator &descriptorAllocator;
  DescriptorWriter &descriptorWriter;
  Buffer<DrawData> const &uniformBuffer;
  std::vector<MeshRenderer const *> singleMeshes;

public:
  MultimeshDrawCommand(Image<2> const &drawImage, Image<2> const &depthImage, DescriptorAllocator &descriptorAllocator,
                       DescriptorWriter &descriptorWriter, Maths::Dimension2 const &renderAreaSize,
                       Buffer<DrawData> const &uniformBuffer, std::vector<MeshRenderer const *> const &meshes)
      : drawImage(drawImage), depthImage(depthImage), singleMeshes(meshes), uniformBuffer(uniformBuffer),
        renderAreaSize(renderAreaSize), descriptorAllocator(descriptorAllocator), descriptorWriter(descriptorWriter) {}
  void QueueExecution(VkCommandBuffer const &) const;
};

void DrawSingleMesh(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                    DescriptorWriter &descriptorWriter, Buffer<DrawData> const &uniformBuffer,
                    MeshRenderer const *renderInfo) {
  AllocatedMesh const *mesh = renderInfo->mesh;
  Material const *material = renderInfo->material;

  // Bind material pipelines
  VkDescriptorSet descriptorSet = descriptorAllocator.Allocate(material->GetDescriptorSetLayout(1));
  material->Bind(commandBuffer, descriptorAllocator, descriptorWriter, uniformBuffer);

  // Upload uniform data
  Maths::Matrix4 model = renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix();
  Maths::Matrix4 normals = renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix().Inverse().Transposed();
  PushConstantsAggregate data{};
  data.PushData(&model);
  mesh->AppendData(data);
  material->AppendData(data);

  vkCmdPushConstants(commandBuffer, material->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, data.Size(),
                     data.Data());

  // Draw mesh
  mesh->BindAndDraw(commandBuffer);
}

void MultimeshDrawCommand::QueueExecution(VkCommandBuffer const &queue) const {
  VkRenderingAttachmentInfo colourAttachmentInfo = drawImage.BindAsColourAttachment();
  VkRenderingAttachmentInfo depthAttachmentInfo = depthImage.BindAsDepthAttachment();

  VkExtent2D drawExtent{renderAreaSize.x(), renderAreaSize.y()};
  VkOffset2D drawOffset{static_cast<int32_t>(renderAreaOffset.x()), static_cast<int32_t>(renderAreaOffset.y())};
  VkRenderingInfo renderingInfo = vkinit::RenderingInfo(colourAttachmentInfo, depthAttachmentInfo, drawExtent);

  VkViewport viewport{.x = static_cast<float>(drawOffset.x),
                      .y = static_cast<float>(drawOffset.y),
                      .width = static_cast<float>(drawExtent.width),
                      .height = static_cast<float>(drawExtent.height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {2 * drawOffset.x, 2 * drawOffset.y},
                   .extent = {drawExtent.width * 20, drawExtent.height * 20}};

  vkCmdSetViewport(queue, 0, 1, &viewport);
  vkCmdSetScissor(queue, 0, 1, &scissor);

  vkCmdBeginRendering(queue, &renderingInfo);

  for (auto mesh : singleMeshes) {
    DrawSingleMesh(queue, descriptorAllocator, descriptorWriter, uniformBuffer, mesh);
  }

  vkCmdEndRendering(queue);
}

std::vector<Engine::Graphics::Command *> Engine::Graphics::RenderingStrategies::ForwardRendering::GetRenderingCommands(
    RenderingRequest const &request, Image<2> &colourImage, Image<2> &depthImage,
    Maths::Dimension2 const &renderDimension, Buffer<DrawData> const &uniformBuffer,
    DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter, Image<2> &swapchainImage) {

  // Commands for drawing geometry
  auto transitionBufferToRenderTarget = colourImage.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  auto transitionBufferToDepthStencil = depthImage.Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  Maths::Matrix4 view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
  Maths::Matrix4 projection = request.camera->projection;

  DrawData uniformData{
      .view = view,
      .projection = projection,
      .viewProjection = projection * view,
      .sceneData = request.sceneData,
  };

  uniformBuffer.SetData(uniformData);
  auto drawMeshes = new MultimeshDrawCommand(colourImage, depthImage, descriptorAllocator, descriptorWriter,
                                             renderDimension, uniformBuffer, request.objectsToDraw);

  // Commands for copying render to swapchain
  auto transitionBufferToTransferSrc = colourImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  auto transitionPresenterToTransferDst = swapchainImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  auto copyBufferToPresenter = colourImage.BlitTo(swapchainImage);

  return {// Draw geometry
          transitionBufferToRenderTarget, transitionBufferToDepthStencil, drawMeshes,

          // Copy render to swapchain
          transitionBufferToTransferSrc, transitionPresenterToTransferDst, copyBufferToPresenter};
}

} // namespace Engine::Graphics::RenderingStrategies