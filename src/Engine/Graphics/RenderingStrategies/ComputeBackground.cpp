#include "ComputeBackground.h"

#include "Graphics/CommandQueue.h"
#include "Graphics/Image.h"
#include "Graphics/VulkanUtil.h"
#include <initializer_list>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics::RenderingStrategies {

std::vector<Graphics::DescriptorAllocator::PoolSizeRatio> ratios{{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

ComputeBackground::ComputeBackground(InstanceManager const *instanceManager, CompiledEffect const *effect,
                                     ComputePushConstants const &data)
    : descriptorAllocator(instanceManager), descriptorWriter(instanceManager), effect(effect), data(data),
      instanceManager(instanceManager) {
  descriptorAllocator.InitPools(10, ratios);

  Graphics::DescriptorLayoutBuilder descriptorLayoutBuilder{instanceManager};
  descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorSetLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
}

void ComputeBackground::RecordRenderingCommands(RenderBuffer renderBuffer, CommandRecorder const &recorder) {

  auto &backgroundTarget = renderBuffer.GetAuxiliaryBuffer(
      renderBuffer.colourImage.GetExtent(), renderBuffer.colourImage->GetFormat(),
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

  auto const targetDescriptor = descriptorAllocator.Allocate(descriptorSetLayout);

  descriptorWriter.WriteImage(0, backgroundTarget, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorWriter.UpdateSet(targetDescriptor);
  descriptorWriter.Clear();

  recorder.RecordTransition(backgroundTarget, VK_IMAGE_LAYOUT_GENERAL);
  recorder.RecordWithBoundPipeline(effect, VK_PIPELINE_BIND_POINT_COMPUTE, [&](MaterialBinder const &binder) {
    binder.RecordDescriptorBind(targetDescriptor);
    binder.RecordPushConstantSet(data, VK_SHADER_STAGE_COMPUTE_BIT);
    binder.RecordDispatch(std::ceil(backgroundTarget.GetExtent().x() / 16u),
                          std::ceil(backgroundTarget.GetExtent().y() / 16u));
  });

  recorder.RecordTransition(backgroundTarget, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  recorder.RecordTransition(renderBuffer.colourImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  recorder.RecordBlit(backgroundTarget, renderBuffer.colourImage, VK_FILTER_LINEAR);
}

void ComputeBackground::Cleanup() {
  descriptorAllocator.ClearDescriptors();
  descriptorAllocator.DestroyPools();
  instanceManager->DestroyDescriptorSetLayout(descriptorSetLayout);
}

} // namespace Engine::Graphics::RenderingStrategies
