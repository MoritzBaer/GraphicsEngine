#include "ComputeBackground.h"

#include "Graphics/VulkanUtil.h"

namespace Engine::Graphics::RenderingStrategies {

class ExecuteComputePipelineCommand : public Command {
  vkutil::BindPipelineCommand bindPipeline;
  vkutil::BindDescriptorSetsCommand bindDescriptors;
  vkutil::DispatchCommand dispatch;
  vkutil::PushConstantsCommand<ComputePushConstants> pushConstants;

public:
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, std::vector<VkDescriptorSet> const &descriptors,
                                ComputePushConstants const &pushConstants, uint32_t workerGroupsX,
                                uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptors),
        dispatch(workerGroupsX, workerGroupsY, workerGroupsZ), pushConstants(pushConstants, layout) {}
  ExecuteComputePipelineCommand(VkPipeline const &pipeline, VkPipelineBindPoint const &bindPoint,
                                VkPipelineLayout const &layout, VkDescriptorSet const &descriptor,
                                ComputePushConstants const &pushConstants, uint32_t workerGroupsX,
                                uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : bindPipeline(pipeline, bindPoint), bindDescriptors(bindPoint, layout, descriptor),
        pushConstants(pushConstants, layout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  ExecuteComputePipelineCommand(CompiledEffect const &effect, ComputePushConstants const &pushData,
                                VkPipelineBindPoint const &bindPoint, VkDescriptorSet const &descriptor,
                                uint32_t workerGroupsX, uint32_t workerGroupsY, uint32_t workerGroupsZ)
      : bindPipeline(effect.pipeline, bindPoint), bindDescriptors(bindPoint, effect.pipelineLayout, descriptor),
        pushConstants(pushData, effect.pipelineLayout), dispatch(workerGroupsX, workerGroupsY, workerGroupsZ) {}
  inline void QueueExecution(VkCommandBuffer const &queue) const {
    bindPipeline.QueueExecution(queue);
    bindDescriptors.QueueExecution(queue);
    pushConstants.QueueExecution(queue);
    dispatch.QueueExecution(queue);
  }
};

std::vector<Graphics::DescriptorAllocator::PoolSizeRatio> ratios{{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

ComputeBackground::ComputeBackground(InstanceManager const *instanceManager, CompiledEffect const &effect,
                                     ComputePushConstants const &data)
    : descriptorAllocator(instanceManager), descriptorWriter(instanceManager), effect(effect), data(data),
      instanceManager(instanceManager) {
  descriptorAllocator.InitPools(10, ratios);

  Graphics::DescriptorLayoutBuilder descriptorLayoutBuilder{instanceManager};
  descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorSetLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
}

ComputeBackground::~ComputeBackground() {
  descriptorAllocator.ClearDescriptors();
  descriptorAllocator.DestroyPools();
  instanceManager->DestroyDescriptorSetLayout(descriptorSetLayout);
}

std::vector<Command *> ComputeBackground::GetRenderingCommands(Maths::Dimension2 const &renderDimension,
                                                               Image<2> &renderTarget) {

  auto targetDescriptor = descriptorAllocator.Allocate(descriptorSetLayout);

  descriptorWriter.WriteImage(0, renderTarget, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  descriptorWriter.UpdateSet(targetDescriptor);
  descriptorWriter.Clear();

  std::vector<Command *> commands{};

  auto transitionBufferToWriteable = renderTarget.Transition(VK_IMAGE_LAYOUT_GENERAL);
  auto computeRun = new ExecuteComputePipelineCommand(effect, data, VK_PIPELINE_BIND_POINT_COMPUTE, targetDescriptor,
                                                      std::ceil<uint32_t>(renderDimension[X] / 16u),
                                                      std::ceil<uint32_t>(renderDimension[Y] / 16u), 1);

  commands.push_back(transitionBufferToWriteable);
  commands.push_back(computeRun);

  return commands;
}

} // namespace Engine::Graphics::RenderingStrategies
