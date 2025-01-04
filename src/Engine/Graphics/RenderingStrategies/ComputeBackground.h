#pragma once

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
  DescriptorAllocator descriptorAllocator;
  DescriptorWriter descriptorWriter;
  VkDescriptorSetLayout descriptorSetLayout;
  CompiledEffect effect;
  ComputePushConstants data;

public:
  ComputeBackground(InstanceManager const *instanceManager, CompiledEffect const &effect,
                    ComputePushConstants const &data);
  ComputeBackground() = default;
  std::vector<Command *> GetRenderingCommands(Maths::Dimension2 const &renderDimension,
                                              Image<2> &renderTarget) override;
};

} // namespace Engine::Graphics::RenderingStrategies

#include "json-parsing.h"
OBJECT_PARSER(Engine::Graphics::RenderingStrategies::ComputePushConstants, FIELD_PARSER(pushData))
