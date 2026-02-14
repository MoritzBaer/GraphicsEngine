#include "ForwardRendering.h"

#include "Graphics/RenderCommand.h"
#include <vulkan/vulkan_core.h>

#define CALCULATE_CLIP_SPACE(name, z)                                                                                  \
  auto name##CamSpace = projection * Vector4(0, 0, z, 1);                                                              \
  auto name = Vector3(name##CamSpace[X], name##CamSpace[Y], name##CamSpace[Z]) / name##CamSpace[W];

namespace Engine::Graphics {

struct DrawData {
  Maths::Matrix4 view;
  Maths::Matrix4 projection;
  Maths::Matrix4 viewProjection;
  SceneData sceneData;
};

template <>
void MultiRenderCommand<MeshRenderer const *>::DoSingleRender(VkCommandBuffer const &commandBuffer,
                                                              MeshRenderer const *const &renderInfo,
                                                              UniformBinding const &uniformBinding) const {
  AllocatedMesh const *mesh = renderInfo->mesh;
  Material const *material = renderInfo->material;

  // Bind material pipelines
  material->Apply(commandBuffer, descriptorAllocator, descriptorWriter, uniformBinding);

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
} // namespace Engine::Graphics

namespace Engine::Graphics::RenderingStrategies {

std::vector<VkFormat> const formatsByPreference = {VK_FORMAT_R16G16B16A16_SFLOAT, //
                                                   VK_FORMAT_R8G8B8A8_SRGB,       //
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
      ENGINE_MESSAGE("Chose format {} for render buffer", (uint64_t)format)
      return format;
    }
  }
  ENGINE_ERROR("No suitable format found for render buffer!")
  return VK_FORMAT_UNDEFINED;
}

void ForwardRendering::CreateRenderBuffer(Maths::Dimension2 const &renderDimension) {
  renderBuffer = {.colourImage = objectManager->AllocateImage(
                      ChooseRenderBufferFormat(), renderDimension, renderBufferUsage,
                      VK_IMAGE_ASPECT_COLOR_BIT DEBUG_LABEL_VALUE(1, 1, VK_SAMPLE_COUNT_1_BIT, "RENDER_BUFFER")),
                  .depthImage = objectManager->CreateDepthBuffer(renderDimension)};
}

void ForwardRendering::DestroyRenderBuffer() {
  objectManager->DestroyImage(renderBuffer.colourImage);
  objectManager->DestroyImage(renderBuffer.depthImage);
}

std::vector<Command const *>
ForwardRendering::GetRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                       DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                                       Image<2> &renderTarget, std::optional<Image<2>> &depthTarget) {

  auto commands = backgroundStrategy->GetRenderingCommands(renderBuffer.colourImage);

  auto transitionBufferToRenderTarget = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  auto transitionBufferToDepthStencil =
      renderBuffer.depthImage.Transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  commands.push_back(transitionBufferToRenderTarget);
  commands.push_back(transitionBufferToDepthStencil);

  Maths::Matrix4 view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
  Maths::Matrix4 projection = request.camera->projection;

  auto v = projection * Vector4(0.01, 1.99, 0.01, 1);
  Vector3 vp = v.xyz();
  auto vh = vp / v[W];
  auto v_ = projection * Vector4(0.01, 2.01, 0.01, 1);
  Vector3 vp_ = v_.xyz();
  auto vh_ = vp_ / v_[W];

  DrawData uniformData{
      .view = view,
      .projection = projection,
      .viewProjection = projection * view,
      .sceneData = request.sceneData,
  };

  auto uniformBinding = uniformBufferProvider.GetBinding<DrawData>(uniformData);
  auto drawMeshes = new MultiRenderCommand<MeshRenderer const *>(
      renderBuffer.colourImage, renderBuffer.depthImage, descriptorAllocator, descriptorWriter,
      renderBuffer.colourImage.GetExtent(), uniformBinding, request.objectsToDraw);

  commands.push_back(drawMeshes);

  // Commands for copying render to target
  auto transitionBufferToTransferSrc = renderBuffer.colourImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  auto transitionTargetToTransferDst = renderTarget.Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  auto copyBufferToPresenter = renderBuffer.colourImage.BlitTo(renderTarget);

  commands.push_back(transitionBufferToTransferSrc);
  commands.push_back(transitionTargetToTransferDst);
  commands.push_back(copyBufferToPresenter);

  // Commands for optionally copying depth to target
  if (depthTarget) {
    auto transitionDepthToTransferSrc = renderBuffer.depthImage.Transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    auto transitionDepthPresenterToTransferDst = depthTarget->Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    auto copyDepthToTarget = renderBuffer.depthImage.BlitTo(*depthTarget);

    commands.push_back(transitionDepthToTransferSrc);
    commands.push_back(transitionDepthPresenterToTransferDst);
    commands.push_back(copyDepthToTarget);
  } else {
    depthTarget.emplace(renderBuffer.depthImage);
  }

  return commands;
}

} // namespace Engine::Graphics::RenderingStrategies