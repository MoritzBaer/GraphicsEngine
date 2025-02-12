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

std::vector<VkFormat> formatsByPreference = {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R16G16B16A16_SNORM,
                                             VK_FORMAT_R8G8B8A8_SNORM};

VkImageUsageFlags renderBufferUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

VkFormat ForwardRendering::ChooseRenderBufferFormat() {
  VkPhysicalDeviceImageFormatInfo2 formatInfo{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
                                              .type = VK_IMAGE_TYPE_2D,
                                              .tiling = VK_IMAGE_TILING_OPTIMAL,
                                              .usage = renderBufferUsage};
  for (auto format : formatsByPreference) {
    formatInfo.format = format;
    if (instanceManager->SupportsFormat(formatInfo)) {
      return format;
    }
  }
  ENGINE_ERROR("No suitable format found for render buffer!")
  return VK_FORMAT_UNDEFINED;
}

void ForwardRendering::CreateRenderBuffer(Maths::Dimension2 const &renderDimension) {
  renderBuffer = {.colourImage = objectManager->CreateAllocatedImage(ChooseRenderBufferFormat(), renderDimension,
                                                                     renderBufferUsage, VK_IMAGE_ASPECT_COLOR_BIT),
                  .depthImage = objectManager->CreateAllocatedImage(VK_FORMAT_D32_SFLOAT, renderDimension,
                                                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                    VK_IMAGE_ASPECT_DEPTH_BIT)};
}

void ForwardRendering::DestroyRenderBuffer() {
  objectManager->DestroyAllocatedImage(renderBuffer.colourImage);
  objectManager->DestroyAllocatedImage(renderBuffer.depthImage);
}

std::vector<Command *> ForwardRendering::GetRenderingCommands(RenderingRequest const &request,
                                                              Buffer<DrawData> const &uniformBuffer,
                                                              DescriptorAllocator &descriptorAllocator,
                                                              DescriptorWriter &descriptorWriter,
                                                              Image<2> &renderTarget) {

  auto commands = backgroundStrategy->GetRenderingCommands(renderBuffer.colourImage);

  auto transitionBufferToRenderTarget = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  auto transitionBufferToDepthStencil =
      renderBuffer.depthImage.Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  commands.push_back(transitionBufferToRenderTarget);
  commands.push_back(transitionBufferToDepthStencil);

  Maths::Matrix4 view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
  Maths::Matrix4 projection = request.camera->projection;

  DrawData uniformData{
      .view = view,
      .projection = projection,
      .viewProjection = projection * view,
      .sceneData = request.sceneData,
  };

  uniformBuffer.SetData(uniformData);
  auto drawMeshes =
      new MultimeshDrawCommand(renderBuffer.colourImage, renderBuffer.depthImage, descriptorAllocator, descriptorWriter,
                               renderTarget.GetExtent(), uniformBuffer, request.objectsToDraw);

  commands.push_back(drawMeshes);

  // Commands for copying render to target
  auto transitionBufferToTransferSrc = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  auto transitionPresenterToTransferDst = renderTarget.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  auto copyBufferToPresenter = renderBuffer.colourImage.BlitTo(renderTarget);

  commands.push_back(transitionBufferToTransferSrc);
  commands.push_back(transitionPresenterToTransferDst);
  commands.push_back(copyBufferToPresenter);

  return commands;
}

} // namespace Engine::Graphics::RenderingStrategies