#pragma once

#include "AssetManager.h"
#include "Graphics/CommandQueue.h"
#include "Graphics/Material.h"
#include "Graphics/RenderingStrategy.h"
#include <vulkan/vulkan_core.h>

namespace Engine::Graphics::RenderingStrategies {

struct CompiledEffect : public Pipeline {
  CompiledEffect(VkPipelineLayout layout, VkPipeline pipeline) : Pipeline(layout, {}, pipeline) {}
};

struct ComputePushConstants {
  std::array<float, 16> pushData;
};

class ComputeBackground : public BackgroundStrategy {
  InstanceManager const *instanceManager;
  VkDescriptorSetLayout descriptorSetLayout;
  CompiledEffect const *effect;
  ComputePushConstants data;

public:
  ComputeBackground(InstanceManager const *instanceManager, CompiledEffect const *effect,
                    ComputePushConstants const &data);
  ComputeBackground() = default;
  ~ComputeBackground() { instanceManager->DestroyDescriptorSetLayout(descriptorSetLayout); };
  void RecordRenderingCommands(DescriptorAllocator &descriptorAllocator, DescriptorWriter &descriptorWriter,
                               RenderBuffer renderBuffer, CommandRecorder const &recorder) override;
};

} // namespace Engine::Graphics::RenderingStrategies

#include "json-parsing.h"
JSON(Engine::Graphics::RenderingStrategies::ComputePushConstants, FIELDS(pushData))
