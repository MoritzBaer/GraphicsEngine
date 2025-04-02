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
  // Dummy implementation // TODO: implement properly
  auto vertexShader = assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(dso.vertexShaderName);
  auto fragmentShader =
      assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(dso.fragmentShaderName);

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder(instanceManager);
  return pipelineBuilder.AddPushConstant<Graphics::ShaderType::VERTEX>(uniformSize, 0)
      .AddDescriptorBinding(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .AddDescriptorBinding(1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      .AddDescriptorBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      .SetShaderStages(vertexShader, fragmentShader)
      .BindSetInShader<Graphics::ShaderType::VERTEX>(0)
      .BindSetInShader<Graphics::ShaderType::FRAGMENT>(0)
      .BindSetInShader<Graphics::ShaderType::FRAGMENT>(1)
      .SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .SetPolygonMode(VK_POLYGON_MODE_FILL)
      .SetColourAttachmentFormat(VK_FORMAT_R16G16B16A16_SNORM) // FIXME: used to be VK_FORMAT_R16G16B16A16_SFLOAT, but
                                                               // that gives a validation error
      .SetDepthFormat(VK_FORMAT_D32_SFLOAT)
      .SetDepthCompareOperation(VK_COMPARE_OP_LESS_OR_EQUAL)
      .EnableBlending(Graphics::PipelineBuilder::BlendMode::ALPHA)
      .SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
      .Build();
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