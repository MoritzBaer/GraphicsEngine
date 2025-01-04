#include "AssetManager.h"

#include "Game.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "Members.h"
#include "json-parsing.h"

template <> struct Engine::AssetManager::AssetDSO<Engine::Graphics::Pipeline *> {
  std::string vertexShaderName;
  std::string fragmentShaderName;
  std::string geometryShaderName;
};

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Graphics::Pipeline *>,
              FIELD_PARSER(vertexShaderName) FIELD_PARSER(fragmentShaderName) FIELD_PARSER(geometryShaderName));

namespace Engine {

// Shaders

template <Graphics::ShaderType Type> struct AssetManager::AssetDSO<Graphics::Shader<Type>> {
  std::string shaderSource;
};

// Paths
template <>
std::string
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::COMPUTE>>::GetAssetPath(char const *assetName) const {
  return std::string("shaders/") + assetName + ".comp";
}
template <>
std::string
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::VERTEX>>::GetAssetPath(char const *assetName) const {
  return std::string("shaders/") + assetName + ".vert";
}
template <>
std::string
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>::GetAssetPath(char const *assetName) const {
  return std::string("shaders/") + assetName + ".geom";
}
template <>
std::string
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>::GetAssetPath(char const *assetName) const {
  return std::string("shaders/") + assetName + ".frag";
}

// Parsing
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::COMPUTE>> *
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::COMPUTE>>::ParseAsset(
    std::string const &assetSource) const {
  return new AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::COMPUTE>>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::FRAGMENT>> *
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>::ParseAsset(
    std::string const &assetSource) const {
  return new AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::GEOMETRY>> *
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>::ParseAsset(
    std::string const &assetSource) const {
  return new AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::VERTEX>> *
AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::VERTEX>>::ParseAsset(
    std::string const &assetSource) const {
  return new AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::VERTEX>>(assetSource);
}

// Conversions
#define SHADER_DSO_CONVERTER(Type)                                                                                     \
  template <>                                                                                                          \
  Graphics::Shader<Graphics::ShaderType::Type>                                                                         \
  AssetManager::AssetLoader<Graphics::Shader<Graphics::ShaderType::Type>>::ConvertDSO(                                 \
      AssetDSO<Graphics::Shader<Graphics::ShaderType::Type>> const *dso) const {                                       \
    return members->shaderCompiler->CompileShaderCode<Graphics::ShaderType::Type>("unnamed shader",                    \
                                                                                  dso->shaderSource.c_str());          \
  }

SHADER_DSO_CONVERTER(COMPUTE)
SHADER_DSO_CONVERTER(VERTEX)
SHADER_DSO_CONVERTER(GEOMETRY)
SHADER_DSO_CONVERTER(FRAGMENT)

template <>
void AssetManager::AssetDestroyer<Graphics::Shader<Graphics::ShaderType::COMPUTE>>::DestroyAsset(
    Graphics::Shader<Graphics::ShaderType::COMPUTE> &asset) const {
  members->shaderCompiler->DestroyShader<Graphics::ShaderType::COMPUTE>(asset);
}
template <>
void AssetManager::AssetDestroyer<Graphics::Shader<Graphics::ShaderType::VERTEX>>::DestroyAsset(
    Graphics::Shader<Graphics::ShaderType::VERTEX> &asset) const {
  members->shaderCompiler->DestroyShader<Graphics::ShaderType::VERTEX>(asset);
}
template <>
void AssetManager::AssetDestroyer<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>::DestroyAsset(
    Graphics::Shader<Graphics::ShaderType::GEOMETRY> &asset) const {
  members->shaderCompiler->DestroyShader<Graphics::ShaderType::GEOMETRY>(asset);
}
template <>
void AssetManager::AssetDestroyer<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>::DestroyAsset(
    Graphics::Shader<Graphics::ShaderType::FRAGMENT> &asset) const {
  members->shaderCompiler->DestroyShader<Graphics::ShaderType::FRAGMENT>(asset);
}

// Pipelines

template <> std::string AssetManager::AssetLoader<Graphics::Pipeline *>::GetAssetPath(char const *assetName) const {
  return std::string("materials/pipelines/") + assetName + ".pl";
}

template <>
AssetManager::AssetDSO<Graphics::Pipeline *> *
AssetManager::AssetLoader<Graphics::Pipeline *>::ParseAsset(std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::Pipeline *>();
  json<AssetDSO<Graphics::Pipeline *>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <>
Graphics::Pipeline *
AssetManager::AssetLoader<Graphics::Pipeline *>::ConvertDSO(AssetDSO<Graphics::Pipeline *> const *dso) const {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader<Graphics::ShaderType::VERTEX> vertexShader =
      members->assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(dso->vertexShaderName);
  Graphics::Shader<Graphics::ShaderType::FRAGMENT> fragmentShader =
      members->assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(dso->fragmentShaderName);

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder(members->instanceManager);
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

template <> void AssetManager::AssetDestroyer<Graphics::Pipeline *>::DestroyAsset(Graphics::Pipeline *&asset) const {
  Graphics::PipelineBuilder::DestroyPipeline(*asset, members->instanceManager);
  delete asset;
}

// Materials

struct MaterialInstanceData {
  virtual void dummy() {} // Dummy function to allow dynamic_cast
};

// AlbedoAndBump

struct AlbedoAndBumpData : public MaterialInstanceData {
  std::string albedoTexture;
  std::string bumpTexture;
  Maths::Vector3 hue{1.0f, 1.0f, 1.0f};
  float specularStrength; // TODO: Extract into Phong
  float phongExponent;
};

// General

template <> struct AssetManager::AssetDSO<Graphics::Material *> {
  std::string pipelineName;
  MaterialInstanceData *instanceData;
};

template <> std::string AssetManager::AssetLoader<Graphics::Material *>::GetAssetPath(char const *assetName) const {
  return std::string("materials/") + assetName + ".mat";
}

template <>
AssetManager::AssetDSO<Graphics::Material *> *
AssetManager::AssetLoader<Graphics::Material *>::ParseAsset(std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::Material *>();
  json<AssetDSO<Graphics::Material *>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <>
Graphics::Material *
AssetManager::AssetLoader<Graphics::Material *>::ConvertDSO(AssetDSO<Graphics::Material *> const *dso) const {
  Graphics::Pipeline const *pipeline = members->assetManager->LoadAsset<Graphics::Pipeline *>(dso->pipelineName);
  if (auto albedoAndBumpDSO = dynamic_cast<AlbedoAndBumpData *>(dso->instanceData)) {
    Graphics::Texture2D albedo = members->assetManager->LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->albedoTexture);
    Graphics::Texture2D bump = members->assetManager->LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->bumpTexture);
    return new Graphics::Materials::AlbedoAndBump(pipeline, albedo, bump, albedoAndBumpDSO->specularStrength,
                                                  albedoAndBumpDSO->phongExponent, albedoAndBumpDSO->hue);
  } else {
    ENGINE_ERROR("Unknown material type!");
    return nullptr;
  }
}

template <> void AssetManager::AssetDestroyer<Graphics::Material *>::DestroyAsset(Graphics::Material *&asset) const {
  delete asset;
}

} // namespace Engine

// JSON stuff

OBJECT_PARSER(Engine::AlbedoAndBumpData, FIELD_PARSER(albedoTexture) FIELD_PARSER(bumpTexture) FIELD_PARSER(hue)
                                             FIELD_PARSER(specularStrength) FIELD_PARSER(phongExponent));

ABSTRACT_OBJECT_PARSER(Engine::MaterialInstanceData, ,
                       INHERITANCE_PARSER(Engine::MaterialInstanceData, Engine::AlbedoAndBumpData));

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Graphics::Material *>,
              FIELD_PARSER(pipelineName) FIELD_PARSER(instanceData) FIELD_PARSER(instanceData));