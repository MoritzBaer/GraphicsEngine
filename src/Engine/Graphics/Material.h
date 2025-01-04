#pragma once

#include "Buffer.h"
#include "DescriptorHandling.h"
#include "DrawData.h"
#include "Shader.h"
#include "UniformAggregate.h"
#include "Util/DeletionQueue.h"
#include "vulkan/vulkan.h"
#include <vector>

namespace Engine::Graphics {

class PipelineBuilder;

class Pipeline {
  friend class PipelineBuilder;

  VkPipeline pipeline;
  VkPipelineLayout layout;
  std::vector<VkDescriptorSetLayout> descriptorLayouts;

public:
  Pipeline(VkPipelineLayout layout, std::vector<VkDescriptorSetLayout> descriptorLayouts, VkPipeline pipeline)
      : pipeline(pipeline), layout(layout), descriptorLayouts(descriptorLayouts) {}
  Pipeline(Pipeline const *other) : Pipeline(other->layout, other->descriptorLayouts, other->pipeline) {}
  Pipeline() = delete;
  inline void Bind(VkCommandBuffer const &commandBuffer) const {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }
  inline VkPipelineLayout Layout() const { return layout; }
  inline VkDescriptorSetLayout DescriptorLayout(uint8_t set) const { return descriptorLayouts[set]; }
};

class Material {
protected:
  Pipeline const *pipeline;

public:
  Material(Material const *other) : pipeline(other->pipeline) {}
  Material(Pipeline const *pipeline) : pipeline(pipeline) {}
  virtual void AppendData(PushConstantsAggregate &aggregate) const = 0;
  virtual void Bind(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                    DescriptorWriter &writer, Buffer<DrawData> const &drawDataBuffer) const {
    pipeline->Bind(commandBuffer);
  }
  VkPipelineLayout GetPipelineLayout() const { return pipeline->Layout(); }
  VkDescriptorSetLayout GetDescriptorSetLayout(uint8_t set) const { return pipeline->DescriptorLayout(set); }
};

class PipelineBuilder {
  struct ShaderStage {
    VkPipelineShaderStageCreateInfo stageInfo;
    VkPushConstantRange pushConstantRange;

    ShaderStage() : stageInfo(), pushConstantRange() {}
  };

  struct DescriptorSet {
    VkDescriptorSetLayout descriptorLayout;
    DescriptorLayoutBuilder layoutBuilder;
    VkShaderStageFlags descriptorSetStages;

    DescriptorSet() : descriptorLayout(), layoutBuilder(nullptr), descriptorSetStages() {}
  };

  InstanceManager const *instanceManager;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineColorBlendAttachmentState colourBlendAttachment;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineLayout pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo depthStencil;
  VkPipelineRenderingCreateInfo renderInfo;
  VkFormat colourAttachmentformat;

  std::array<VkPipelineShaderStageCreateInfo, static_cast<size_t>(ShaderType::NUMBER_OF_TYPES)> shaderStageInfos;
  std::vector<VkPushConstantRange> pushConstantRanges;
  std::vector<DescriptorSet> descriptorSets;

  inline void SetBlendFactors(VkBlendFactor const &srcFactor, VkBlendFactor const &dstFactor);

public:
  enum class BlendMode { ALPHA, ADDITIVE };
  PipelineBuilder &Reset();

  PipelineBuilder(InstanceManager const *instanceManager)
      : instanceManager(instanceManager), descriptorSets(4), shaderStageInfos(), pushConstantRanges() {
    for (int i = 0; i < 4; i++) {
      descriptorSets[i].layoutBuilder = DescriptorLayoutBuilder(instanceManager);
    }
    Reset();
  }

  PipelineBuilder &SetShaderStages(Graphics::Shader<Graphics::ShaderType::VERTEX> const &vertexShader,
                                   Graphics::Shader<Graphics::ShaderType::FRAGMENT> const &fragmentShader);
  PipelineBuilder &SetInputTopology(VkPrimitiveTopology const &topology);
  PipelineBuilder &SetPolygonMode(VkPolygonMode const &polygonMode);
  PipelineBuilder &SetCullMode(VkCullModeFlags const &cullMode, VkFrontFace const &frontFace);
  PipelineBuilder &SetColourAttachmentFormat(VkFormat const &format);
  PipelineBuilder &SetDepthFormat(VkFormat const &format);
  PipelineBuilder &DisableDepthTest();
  PipelineBuilder &DisableDepthWriting();
  PipelineBuilder &SetDepthCompareOperation(VkCompareOp const &compareOp);
  PipelineBuilder &EnableBlending(BlendMode const &mode);
  PipelineBuilder &AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType descriptorType);
  template <ShaderType Type> PipelineBuilder &AddPushConstant(size_t size, size_t offset);
  template <ShaderType Type> PipelineBuilder &BindSetInShader(uint8_t set);

  Pipeline *Build();

  static void DestroyPipeline(Pipeline const &pipeline, InstanceManager const *im);
};

template <ShaderType Type> inline PipelineBuilder &PipelineBuilder::BindSetInShader(uint8_t set) {
  descriptorSets[set].descriptorSetStages = descriptorSets[set].descriptorSetStages | StageConstants<Type>::stageFlags;
  return *this;
}

template <ShaderType Type> inline PipelineBuilder &PipelineBuilder::AddPushConstant(size_t size, size_t offset) {
  pushConstantRanges.push_back(
      VkPushConstantRange{.stageFlags = static_cast<VkShaderStageFlags>(StageConstants<Type>::stageFlags),
                          .offset = static_cast<uint32_t>(offset),
                          .size = static_cast<uint32_t>(size)});
  return *this;
}

} // namespace Engine::Graphics
