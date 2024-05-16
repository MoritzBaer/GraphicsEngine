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

class Pipeline : public ConstDestroyable {
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
  void Destroy() const;
};

class Material {
protected:
  Pipeline const *pipeline;

public:
  Material(Material const *other) : pipeline(other->pipeline) {}
  Material(Pipeline const *pipeline) : pipeline(pipeline) {}
  virtual void AppendData(PushConstantsAggregate &aggregate) const = 0;
  virtual void Bind(VkCommandBuffer const &commandBuffer, DescriptorAllocator &descriptorAllocator,
                    Buffer<DrawData> const &drawDataBuffer) const {
    pipeline->Bind(commandBuffer);
  }
  VkPipelineLayout GetPipelineLayout() const { return pipeline->Layout(); }
  VkDescriptorSetLayout GetDescriptorSetLayout(uint8_t set) const { return pipeline->DescriptorLayout(set); }
};

class PipelineBuilder {
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineColorBlendAttachmentState colourBlendAttachment;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineLayout pipelineLayout;
  VkPipelineDepthStencilStateCreateInfo depthStencil;
  VkPipelineRenderingCreateInfo renderInfo;
  VkFormat colourAttachmentformat;

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  std::vector<VkPushConstantRange> pushConstantRanges;

  std::vector<DescriptorLayoutBuilder> layoutBuilders;
  std::vector<VkShaderStageFlags> descriptorSetStages;

  inline void SetBlendFactors(VkBlendFactor const &srcFactor, VkBlendFactor const &dstFactor);

public:
  enum class BlendMode { ALPHA, ADDITIVE };
  PipelineBuilder &Reset();

  PipelineBuilder() : layoutBuilders(4), descriptorSetStages(4) { Reset(); }

  PipelineBuilder &SetShaderStages(Graphics::Shader const &vertexShader, Graphics::Shader const &fragmentShader);
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
  PipelineBuilder &AddPushConstant(size_t size, size_t offset, ShaderType shaderType);
  template <typename... ShaderTypes> PipelineBuilder &BindSetInShaders(uint8_t set, ShaderTypes... shaderTypes);

  Pipeline *Build();
};

template <typename... ShaderTypes>
inline PipelineBuilder &PipelineBuilder::BindSetInShaders(uint8_t set, ShaderTypes... shaderTypes) {
  descriptorSetStages[set] = descriptorSetStages[set] | (ConvertToShaderStage(shaderTypes) | ...);
  return *this;
}

} // namespace Engine::Graphics
