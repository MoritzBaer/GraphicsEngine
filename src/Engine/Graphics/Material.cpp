#include "Material.h"

#include "InstanceManager.h"

namespace Engine::Graphics {

void PipelineBuilder::SetBlendFactors(VkBlendFactor const &srcFactor, VkBlendFactor const &dstFactor) {
  colourBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colourBlendAttachment.blendEnable = VK_TRUE;
  colourBlendAttachment.srcColorBlendFactor = srcFactor;
  colourBlendAttachment.dstColorBlendFactor = dstFactor;
  colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

PipelineBuilder &PipelineBuilder::Reset() {
  inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                   .primitiveRestartEnable = VK_FALSE};

  rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f};

  colourBlendAttachment = {};

  multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                   .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                   .sampleShadingEnable = VK_FALSE,
                   .minSampleShading = 1.0f,
                   .alphaToCoverageEnable = VK_FALSE,
                   .alphaToOneEnable = VK_FALSE};

  colourBlendAttachment = {.blendEnable = VK_FALSE,
                           .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  pipelineLayout = {};
  depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                  .depthTestEnable = VK_TRUE,
                  .depthWriteEnable = VK_TRUE,
                  .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                  .depthBoundsTestEnable = VK_FALSE,
                  .stencilTestEnable = VK_FALSE,
                  .front = {},
                  .back = {},
                  .minDepthBounds = 0.f,
                  .maxDepthBounds = 1.f};

  renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &colourAttachmentformat};

  for (auto &shaderStage : shaderStageInfos) {
    shaderStage = {};
  }
  for (auto &descriptor : descriptorSets) {
    descriptor.layoutBuilder.Clear();
  }

  return *this;
}

PipelineBuilder &
PipelineBuilder::SetShaderStages(Graphics::Shader<Graphics::ShaderType::VERTEX> const *vertexShader,
                                 Graphics::Shader<Graphics::ShaderType::FRAGMENT> const *fragmentShader) {
  shaderStageInfos[static_cast<size_t>(ShaderType::VERTEX)] = vertexShader->GetStageInfo();
  shaderStageInfos[static_cast<size_t>(ShaderType::FRAGMENT)] = fragmentShader->GetStageInfo();
  return *this;
}

PipelineBuilder &PipelineBuilder::SetInputTopology(VkPrimitiveTopology const &topology) {
  inputAssembly.topology = topology;
  return *this;
}

PipelineBuilder &PipelineBuilder::SetPolygonMode(VkPolygonMode const &polygonMode) {
  rasterizer.polygonMode = polygonMode;
  return *this;
}

PipelineBuilder &PipelineBuilder::SetCullMode(VkCullModeFlags const &cullMode, VkFrontFace const &frontFace) {
  rasterizer.cullMode = cullMode;
  rasterizer.frontFace = frontFace;
  return *this;
}

PipelineBuilder &PipelineBuilder::SetColourAttachmentFormat(VkFormat const &format) {
  colourAttachmentformat = format;
  return *this;
}

PipelineBuilder &PipelineBuilder::SetDepthFormat(VkFormat const &format) {
  renderInfo.depthAttachmentFormat = format;
  return *this;
}

PipelineBuilder &PipelineBuilder::DisableDepthTest() {
  depthStencil.depthTestEnable = VK_FALSE;
  depthStencil.depthWriteEnable = VK_FALSE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;
  return *this;
}

PipelineBuilder &PipelineBuilder::DisableDepthWriting() {
  depthStencil.depthWriteEnable = VK_FALSE;
  return *this;
}

PipelineBuilder &PipelineBuilder::SetDepthCompareOperation(VkCompareOp const &compareOp) {
  depthStencil.depthCompareOp = compareOp;
  return *this;
}

PipelineBuilder &PipelineBuilder::EnableBlending(BlendMode const &mode) {
  switch (mode) {
  case BlendMode::ALPHA:
    SetBlendFactors(VK_BLEND_FACTOR_DST_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
    break;
  case BlendMode::ADDITIVE:
    SetBlendFactors(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_DST_ALPHA);
    break;
  }
  return *this;
}

PipelineBuilder &PipelineBuilder::AddDescriptorBinding(uint32_t set, uint32_t binding,
                                                       VkDescriptorType descriptorType) {
  // TODO: Allow multiple shader stages
  if (set >= descriptorSets.size()) {
    auto oldSetCount = descriptorSets.size();
    descriptorSets.resize(set + 1);
    for (auto i = oldSetCount; i < descriptorSets.size(); i++) {
      descriptorSets[i].layoutBuilder = DescriptorLayoutBuilder(instanceManager);
    }
  }
  descriptorSets[set].layoutBuilder.AddBinding(binding, descriptorType);
  return *this;
}

Pipeline *PipelineBuilder::Build() {
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
  for (uint32_t i = 0; i < descriptorSets.size(); i++) {
    if (descriptorSets[i].layoutBuilder.HasBindings()) {
      descriptorSetLayouts.push_back(descriptorSets[i].layoutBuilder.Build(descriptorSets[i].descriptorSetStages));
    }
  }

  VkPipelineLayout pipelineLayout;
  VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                        .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
                                        .pSetLayouts = descriptorSetLayouts.data(),
                                        .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
                                        .pPushConstantRanges = pushConstantRanges.data()};
  instanceManager->CreatePipelineLayout(&layoutInfo, &pipelineLayout);

  VkPipelineViewportStateCreateInfo viewportInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

  VkPipelineColorBlendStateCreateInfo colourBlendInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                      .logicOpEnable = VK_FALSE,
                                                      .attachmentCount = 1,
                                                      .pAttachments = &colourBlendAttachment};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{.sType =
                                                           VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

  VkDynamicState states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = states};

  std::vector<VkPipelineShaderStageCreateInfo> activeStages{};
  for (auto const &stage : shaderStageInfos) {
    if (stage.sType)
      activeStages.push_back(stage);
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                            .pNext = &renderInfo,
                                            .stageCount = static_cast<uint32_t>(activeStages.size()),
                                            .pStages = activeStages.data(),
                                            .pVertexInputState = &vertexInputInfo,
                                            .pInputAssemblyState = &inputAssembly,
                                            .pViewportState = &viewportInfo,
                                            .pRasterizationState = &rasterizer,
                                            .pMultisampleState = &multisampling,
                                            .pDepthStencilState = &depthStencil,
                                            .pColorBlendState = &colourBlendInfo,
                                            .pDynamicState = &dynamicStateInfo,
                                            .layout = pipelineLayout};

  VkPipeline pipeline;
  instanceManager->CreateGraphicsPipeline(pipelineInfo, &pipeline);

  return new Pipeline(pipelineLayout, descriptorSetLayouts, pipeline);
}

void PipelineBuilder::DestroyPipeline(Pipeline const &pipeline, InstanceManager const *instanceManager) {
  instanceManager->DestroyPipeline(pipeline.pipeline);
  instanceManager->DestroyPipelineLayout(pipeline.layout);
  for (auto layout : pipeline.descriptorLayouts) {
    instanceManager->DestroyDescriptorSetLayout(layout);
  }
}

} // namespace Engine::Graphics