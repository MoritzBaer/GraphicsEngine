#include "MaterialParsing.h"

#include "AssetManager.h"
#include "Game.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "Util/AssetParsing/MaterialParsing.h"
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
    auto vertexStage =
        assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(dso.vertexStage.shaderName);
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
    auto fragmentStage =
        assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(dso.fragmentStage.shaderName);
    pipelineBuilder.SetShaderStage<Graphics::ShaderType::FRAGMENT>(fragmentStage);
    for (auto const &pushConstant : dso.fragmentStage.pushConstants) {
      pipelineBuilder.AddPushConstant<Graphics::ShaderType::FRAGMENT>(pushConstant.size, pushConstant.offset);
    }
    for (auto const &set : dso.fragmentStage.boundDescriptorSets) {
      pipelineBuilder.BindSetInShader<Graphics::ShaderType::FRAGMENT>(set);
    }
  }

  if (!dso.geometryStage.shaderName.empty()) {
    auto geometryStage =
        assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>(dso.geometryStage.shaderName);
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

struct AlbedoAndBumpData : public MaterialInstanceData {
  std::string albedoTexture;
  std::string bumpTexture;
  Maths::Vector3 hue{1.0f, 1.0f, 1.0f};
  float specularStrength; // TODO: Extract into Phong
  float phongExponent;
};

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

void specialize(AssetManager * am, Graphics::InstanceManager const * im) { // To force json template specialization
  // TODO: Find a better way of doing this! Once found, use for asset loading elsewhere
  MaterialManager mm = MaterialManager(MaterialLoader(am), MaterialCache()); 
  mm.LoadAsset("test");
  PipelineManager pm = PipelineManager(PipelineLoader(am, im), PipelineCache(im));
  pm.LoadAsset("test");
}

} // namespace Engine

JSON_ENUM(VkPrimitiveTopology, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
          VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
          VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
          VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
JSON_ENUM(VkPolygonMode, VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT,
          VK_POLYGON_MODE_FILL_RECTANGLE_NV);
JSON_ENUM(VkFrontFace, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FRONT_FACE_CLOCKWISE);
JSON_ENUM(
    VkFormat, VK_FORMAT_UNDEFINED, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8_UNORM, VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT,
    VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_R8_UINT) // Add more formats if/when needed. Can't do all because that would be too much block nesting.
JSON_ENUM(VkCompareOp, VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL,
          VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS)
JSON_ENUM(VkDescriptorType, VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
          VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
          VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM, VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM,
          VK_DESCRIPTOR_TYPE_MUTABLE_EXT, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, VK_DESCRIPTOR_TYPE_MUTABLE_VALVE)
JSON_ENUM(VkVertexInputRate, VK_VERTEX_INPUT_RATE_VERTEX, VK_VERTEX_INPUT_RATE_INSTANCE)
JSON(Engine::PushConstant, FIELDS(size, offset))
JSON(Engine::ShaderStage, FIELDS(shaderName, pushConstants, boundDescriptorSets))
JSON(Engine::DescriptorBinding, FIELDS(set, binding, descriptorType));
JSON(Engine::VertexInputAttribute, FIELDS(location, binding, format, offset))
JSON(Engine::VertexInputBinding, FIELDS(binding, stride, inputRate, attributes))

JSON(Engine::PipelineDSO, FIELDS(vertexStage, fragmentStage, geometryStage, topology, polygonMode, cullMode, frontFace,
                                 colourAttachmentFormat, depthFormat, depthTestEnabled, depthWriteEnabled,
                                 depthCompareOperation, blendingEnabled, descriptorBindings, vertexInputBindings));

JSON(Engine::MaterialDSO, FIELDS(pipelineName, instanceData));

JSON(Engine::AlbedoAndBumpData, FIELDS(albedoTexture, bumpTexture, hue, specularStrength, phongExponent));

JSON(Engine::MaterialInstanceData *, SUBTYPES(Engine::AlbedoAndBumpData));