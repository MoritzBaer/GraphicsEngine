#include "ComputeBackground.h"

#include "Graphics/CommandQueue.h"
#include "Graphics/VulkanUtil.h"
#include <initializer_list>
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics::RenderingStrategies {

std::vector<Graphics::DescriptorAllocator::PoolSizeRatio> ratios{{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

ComputeBackground::ComputeBackground(InstanceManager const *instanceManager, Pipeline const &effect,
                                     ComputePushConstants const &data)
    : descriptorAllocator(instanceManager), descriptorWriter(instanceManager), effect(effect), data(data),
      instanceManager(instanceManager) {
  descriptorAllocator.InitPools(10, ratios);

  Graphics::DescriptorLayoutBuilder descriptorLayoutBuilder{instanceManager};
  descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorSetLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
}

void ComputeBackground::RecordRenderingCommands(Image<2> &renderTarget, CommandRecorder const &recorder) {

  auto const targetDescriptor = descriptorAllocator.Allocate(descriptorSetLayout);

  descriptorWriter.WriteImage(0, renderTarget, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorWriter.UpdateSet(targetDescriptor);
  descriptorWriter.Clear();

  recorder.RecordWithBoundPipeline(effect, VK_PIPELINE_BIND_POINT_COMPUTE, [&](MaterialBinder const &binder) {
    binder.RecordDescriptorBind(targetDescriptor);
    binder.RecordPushConstantSet(data, VK_SHADER_STAGE_COMPUTE_BIT);
    binder.RecordDispatch(std::ceil(renderTarget.GetExtent().x() / 16u), std::ceil(renderTarget.GetExtent().y() / 16u));
  });

  recorder.RecordTransition(renderTarget, VK_IMAGE_LAYOUT_GENERAL);
}

void ComputeBackground::Cleanup() {
  descriptorAllocator.ClearDescriptors();
  descriptorAllocator.DestroyPools();
  instanceManager->DestroyDescriptorSetLayout(descriptorSetLayout);
}

} // namespace Engine::Graphics::RenderingStrategies
