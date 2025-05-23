#pragma once

#include "AssetManager.h"
#include "Graphics/RenderingStrategy.h"

namespace Engine::Graphics::RenderingStrategies {
struct CompiledEffect {
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline;
};

struct ComputePushConstants {
  std::array<float, 16> pushData;
};

class ComputeBackground : public BackgroundStrategy {
  InstanceManager const *instanceManager;
  DescriptorAllocator descriptorAllocator;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout descriptorSetLayout;
  CompiledEffect effect;
  ComputePushConstants data;

public:
  ComputeBackground(InstanceManager const *instanceManager, CompiledEffect const &effect,
                    ComputePushConstants const &data);
  ComputeBackground() = default;
  std::vector<Command *> GetRenderingCommands(Image<2> &renderTarget) override;
  void Cleanup();
};

} // namespace Engine::Graphics::RenderingStrategies

#include "json-parsing.h"
JSON(Engine::Graphics::RenderingStrategies::ComputePushConstants, FIELDS(pushData))
