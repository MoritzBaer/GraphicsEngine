#pragma once

#include "AssetManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/Material.h"
#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {

struct CompiledEffect {
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
};

struct ComputePushConstants {
  std::array<float, 16> pushData;
};

class ComputeBackground : public BackgroundStrategy {
  InstanceManager const *instanceManager;
  DescriptorAllocator descriptorAllocator;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout descriptorSetLayout;
  Pipeline effect;
  ComputePushConstants data;

public:
  ComputeBackground(InstanceManager const *instanceManager, Pipeline const &effect, ComputePushConstants const &data);
  ComputeBackground() = default;
  void RecordRenderingCommands(Image<2> &renderTarget, CommandRecorder const &recorder) override;
  void Cleanup();
};

} // namespace Engine::Graphics::RenderingStrategies

#include "json-parsing.h"
JSON(Engine::Graphics::RenderingStrategies::ComputePushConstants, FIELDS(pushData))
