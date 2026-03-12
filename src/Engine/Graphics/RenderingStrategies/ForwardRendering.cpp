#include "ForwardRendering.h"

#include "AssetManager.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/UniformAggregate.h"
#include "Graphics/VulkanUtil.h"
#include <cstdlib>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics::RenderingStrategies {

struct DrawData {
  Maths::Matrix4 view;
  Maths::Matrix4 projection;
  Maths::Matrix4 viewProjection;
  SceneData sceneData;
};

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
  auto fenceInfo = vkinit::FenceCreateInfo();
  instanceManager->CreateFence(&fenceInfo, &renderBuffer.renderFence);
}

void ForwardRendering::DestroyRenderBuffer() {
  objectManager->DestroyImage(renderBuffer.colourImage);
  objectManager->DestroyImage(renderBuffer.depthImage);
}

void ForwardRendering::RecordRenderingCommands(RenderingRequest const &request, UniformBinder &uniformBufferProvider,
                                               DescriptorAllocator &descriptorAllocator,
                                               DescriptorWriter &descriptorWriter, Image<2> &renderTarget,
                                               std::optional<Image<2>> &depthTarget, CommandRecorder const &recorder) {

  instanceManager->WaitForFences(&renderBuffer.renderFence);
  instanceManager->ResetFences(&renderBuffer.renderFence);

  backgroundStrategy->RecordRenderingCommands(renderBuffer.colourImage, recorder);

  recorder.RecordTransition(renderBuffer.colourImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  recorder.RecordTransition(renderBuffer.depthImage, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  // Draw meshes in perspective
  Maths::Matrix4 const view = request.camera->entity.GetComponent<Transform>()->WorldToModelMatrix();
  Maths::Matrix4 const projection = request.camera->projection;

  DrawData const uniformData{
      .view = view,
      .projection = projection,
      .viewProjection = projection * view,
      .sceneData = request.sceneData,
  };

  auto const uniformBinding = uniformBufferProvider.GetBinding<DrawData>(uniformData);

  recorder.RecordViewports(vkutil::MakeViewport(renderBuffer.colourImage.GetExtent()));
  recorder.RecordScissors(vkutil::MakeRect(renderBuffer.colourImage.GetExtent()));
  recorder.RecordRenderPass()
      .WithDrawImage(renderBuffer.colourImage)
      .WithDepthImage(renderBuffer.depthImage)
      .As([&](RenderPassRecorder const &recorder) {
        for (auto const &renderInfo : request.objectsToDraw) {

          // Draw indivisual mesh
          AllocatedMesh const &mesh = renderInfo->mesh;
          Material const &material = renderInfo->material;
          auto const descriptors = material.WriteDescriptors(descriptorAllocator, descriptorWriter, uniformBinding);

          auto const model = renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix();
          auto const normals =
              renderInfo->entity.GetComponent<Transform>()->ModelToWorldMatrix().Inverse().Transposed();
          PushConstantsAggregate data{};
          data.PushData(&model);
          mesh.AppendData(data);
          material.AppendData(data);

          recorder.RecordWithBoundPipeline(material.GetPipeline(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                           [&](DrawCallRecorder const &recorder) {
                                             recorder.RecordDescriptorBind(std::span(descriptors));
                                             recorder.RecordPushConstantSet(data, VK_SHADER_STAGE_VERTEX_BIT);
                                             recorder.RecordMeshDraw(mesh);
                                           });
        }
      });

  // Commands for copying render to target
  recorder.RecordTransition(renderBuffer.colourImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  recorder.RecordTransition(renderTarget, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  recorder.RecordBlit(renderBuffer.colourImage, renderTarget, VK_FILTER_LINEAR);

  // Commands for optionally copying depth to target
  if (depthTarget) {
    recorder.RecordTransition(renderBuffer.depthImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    recorder.RecordTransition(depthTarget.value(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    recorder.RecordBlit(renderBuffer.depthImage, depthTarget.value(), VK_FILTER_NEAREST);
  } else {
    depthTarget.emplace(renderBuffer.depthImage);
  }
}

} // namespace Engine::Graphics::RenderingStrategies