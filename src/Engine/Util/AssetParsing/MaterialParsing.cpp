#include "MaterialParsing.h"

#include "Game.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "json-parsing.h"

namespace Engine {

// Pipelines

template <> std::string AssetPath<Graphics::Pipeline *>::FromName(char const *assetName) {
  return std::string("materials/pipelines/") + assetName + ".pl";
}

Graphics::Pipeline *PipelineConverter::ConvertDSO(PipelineDSO const &dso) const {
  Graphics::PipelineBuilder pipelineBuilder(instanceManager);
  pipelineBuilder.SetInputTopology(dso.topology)
      .SetPolygonMode(dso.polygonMode)
      .SetCullMode(dso.cullMode, dso.frontFace)
      .SetColourAttachmentFormat(dso.colourAttachmentFormat)
      .SetDepthFormat(dso.depthFormat)
      .SetDepthCompareOperation(dso.depthCompareOperation);
  if (!dso.depthTestEnabled) {
    pipelineBuilder.DisableDepthTest();
  }
  if (!dso.depthWriteEnabled) {
    pipelineBuilder.DisableDepthWriting();
  }
  if (dso.blendingEnabled) {
    pipelineBuilder.EnableBlending(Graphics::PipelineBuilder::BlendMode::ALPHA);
  }

  for (auto const &binding : dso.descriptorBindings) {
    pipelineBuilder.AddDescriptorBinding(binding.set, binding.binding, binding.descriptorType);
  }

  if (!dso.vertexStage.shaderName.empty()) {
    auto vertexStage = assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(dso.vertexStage.shaderName);
    pipelineBuilder.SetShaderStage<Graphics::ShaderType::VERTEX>(vertexStage);
    for (auto const &pushConstant : dso.vertexStage.pushConstants) {
      pipelineBuilder.AddPushConstant<Graphics::ShaderType::VERTEX>(pushConstant.size, pushConstant.offset);
    }
    for (auto const &set : dso.vertexStage.boundDescriptorSets) {
      pipelineBuilder.BindSetInShader<Graphics::ShaderType::VERTEX>(set);
    }
  }

  for (auto const &binding : dso.vertexInputBindings) {
    pipelineBuilder.AddVertexInputBinding(binding.binding, binding.stride, binding.inputRate);
    for (auto const &attribute : binding.attributes) {
      pipelineBuilder.AddVertexInputAttribute(attribute.location, attribute.binding, attribute.format,
                                                     attribute.offset);
    }
  }

  if (!dso.fragmentStage.shaderName.empty()) {
    auto fragmentStage = assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(dso.fragmentStage.shaderName);
    pipelineBuilder.SetShaderStage<Graphics::ShaderType::FRAGMENT>(fragmentStage);
    for (auto const &pushConstant : dso.fragmentStage.pushConstants) {
      pipelineBuilder.AddPushConstant<Graphics::ShaderType::FRAGMENT>(pushConstant.size, pushConstant.offset);
    }
    for (auto const &set : dso.fragmentStage.boundDescriptorSets) {
      pipelineBuilder.BindSetInShader<Graphics::ShaderType::FRAGMENT>(set);
    }
  }

  if (!dso.geometryStage.shaderName.empty()) {
    auto geometryStage = assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>(dso.geometryStage.shaderName);
    pipelineBuilder.SetShaderStage<Graphics::ShaderType::GEOMETRY>(geometryStage);
    for (auto const &pushConstant : dso.geometryStage.pushConstants) {
      pipelineBuilder.AddPushConstant<Graphics::ShaderType::GEOMETRY>(pushConstant.size, pushConstant.offset);
    }
    for (auto const &set : dso.geometryStage.boundDescriptorSets) {
      pipelineBuilder.BindSetInShader<Graphics::ShaderType::GEOMETRY>(set);
    }
  }
  return pipelineBuilder.Build();
}

void PipelineDestroyer::DestroyAsset(Graphics::Pipeline *&asset) const {
  Graphics::PipelineBuilder::DestroyPipeline(*asset, instanceManager);
  delete asset;
}

// +-----------+
// | MATERIALS |
// +-----------+

// General

template <> std::string AssetPath<Graphics::Material *>::FromName(char const *assetName) {
  return std::string("materials/") + assetName + ".mat";
}

Graphics::Material *MaterialConverter::ConvertDSO(MaterialDSO const &dso) const {
  Graphics::Pipeline const *pipeline = assetManager->LoadAsset<Graphics::Pipeline *>(dso.pipelineName);
  Graphics::Material *material = nullptr;
  if (auto albedoAndBumpDSO = dynamic_cast<AlbedoAndBumpData *>(dso.instanceData)) {
    Graphics::Texture2D albedo = assetManager->LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->albedoTexture);
    Graphics::Texture2D bump = assetManager->LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->bumpTexture);
    material = new Graphics::Materials::AlbedoAndBump(pipeline, albedo, bump, albedoAndBumpDSO->specularStrength,
                                                      albedoAndBumpDSO->phongExponent, albedoAndBumpDSO->hue);
  } else {
    ENGINE_ERROR("Unknown material type!");
  }

  delete dso.instanceData;
  return material;
}

} // namespace Engine