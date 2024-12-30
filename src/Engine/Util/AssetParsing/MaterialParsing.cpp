#include "AssetManager.h"

#include "Game.h"
#include "Graphics/Materials/AlbedoAndBump.h"
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
std::string AssetManager::GetAssetPath<Graphics::Shader<Graphics::ShaderType::COMPUTE>>(char const *assetName) const {
  return std::string("shaders/") + assetName + ".comp";
}
template <>
std::string AssetManager::GetAssetPath<Graphics::Shader<Graphics::ShaderType::VERTEX>>(char const *assetName) const {
  return std::string("shaders/") + assetName + ".vert";
}
template <>
std::string AssetManager::GetAssetPath<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>(char const *assetName) const {
  return std::string("shaders/") + assetName + ".geom";
}
template <>
std::string AssetManager::GetAssetPath<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(char const *assetName) const {
  return std::string("shaders/") + assetName + ".frag";
}

// Parsing
template <Graphics::ShaderType Type>
AssetManager::AssetDSO<Graphics::Shader<Type>> *ParseShader(std::string const &assetSource) {
  return new AssetManager::AssetDSO<Graphics::Shader<Type>>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::COMPUTE>> *
AssetManager::ParseAsset<Graphics::Shader<Graphics::ShaderType::COMPUTE>>(std::string const &assetSource) const {
  return ParseShader<Graphics::ShaderType::COMPUTE>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::FRAGMENT>> *
AssetManager::ParseAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(std::string const &assetSource) const {
  return ParseShader<Graphics::ShaderType::FRAGMENT>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::GEOMETRY>> *
AssetManager::ParseAsset<Graphics::Shader<Graphics::ShaderType::GEOMETRY>>(std::string const &assetSource) const {
  return ParseShader<Graphics::ShaderType::GEOMETRY>(assetSource);
}
template <>
AssetManager::AssetDSO<Graphics::Shader<Graphics::ShaderType::VERTEX>> *
AssetManager::ParseAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(std::string const &assetSource) const {
  return ParseShader<Graphics::ShaderType::VERTEX>(assetSource);
}

// Conversions
template <>
Graphics::Shader<Graphics::ShaderType::COMPUTE>
AssetManager::ConvertDSO(AssetDSO<Graphics::Shader<Graphics::ShaderType::COMPUTE>> const *dso) {
  return game->shaderCompiler.CompileShaderCode<Graphics::ShaderType::COMPUTE>("unnamed shader",
                                                                               dso->shaderSource.c_str());
}
template <>
Graphics::Shader<Graphics::ShaderType::VERTEX>
AssetManager::ConvertDSO(AssetDSO<Graphics::Shader<Graphics::ShaderType::VERTEX>> const *dso) {
  return game->shaderCompiler.CompileShaderCode<Graphics::ShaderType::VERTEX>("unnamed shader",
                                                                              dso->shaderSource.c_str());
}
template <>
Graphics::Shader<Graphics::ShaderType::GEOMETRY>
AssetManager::ConvertDSO(AssetDSO<Graphics::Shader<Graphics::ShaderType::GEOMETRY>> const *dso) {
  return game->shaderCompiler.CompileShaderCode<Graphics::ShaderType::GEOMETRY>("unnamed shader",
                                                                                dso->shaderSource.c_str());
}
template <>
Graphics::Shader<Graphics::ShaderType::FRAGMENT>
AssetManager::ConvertDSO(AssetDSO<Graphics::Shader<Graphics::ShaderType::FRAGMENT>> const *dso) {
  return game->shaderCompiler.CompileShaderCode<Graphics::ShaderType::FRAGMENT>("unnamed shader",
                                                                                dso->shaderSource.c_str());
}

template <> void AssetManager::DestroyAsset(Graphics::Shader<Graphics::ShaderType::COMPUTE> &asset) const {
  game->shaderCompiler.DestroyShader<Graphics::ShaderType::COMPUTE>(asset);
}
template <> void AssetManager::DestroyAsset(Graphics::Shader<Graphics::ShaderType::VERTEX> &asset) const {
  game->shaderCompiler.DestroyShader<Graphics::ShaderType::VERTEX>(asset);
}
template <> void AssetManager::DestroyAsset(Graphics::Shader<Graphics::ShaderType::GEOMETRY> &asset) const {
  game->shaderCompiler.DestroyShader<Graphics::ShaderType::GEOMETRY>(asset);
}
template <> void AssetManager::DestroyAsset(Graphics::Shader<Graphics::ShaderType::FRAGMENT> &asset) const {
  game->shaderCompiler.DestroyShader<Graphics::ShaderType::FRAGMENT>(asset);
}

// Pipelines

template <> std::string AssetManager::GetAssetPath<Graphics::Pipeline *>(char const *assetName) const {
  return std::string("materials/pipelines/") + assetName + ".pl";
}

template <>
AssetManager::AssetDSO<Graphics::Pipeline *> *
AssetManager::ParseAsset<Graphics::Pipeline *>(std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::Pipeline *>();
  json<AssetDSO<Graphics::Pipeline *>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <>
Graphics::Pipeline *AssetManager::ConvertDSO<Graphics::Pipeline *>(AssetDSO<Graphics::Pipeline *> const *dso) {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader<Graphics::ShaderType::VERTEX> vertexShader =
      LoadAsset<Graphics::Shader<Graphics::ShaderType::VERTEX>>(dso->vertexShaderName);
  Graphics::Shader<Graphics::ShaderType::FRAGMENT> fragmentShader =
      LoadAsset<Graphics::Shader<Graphics::ShaderType::FRAGMENT>>(dso->fragmentShaderName);

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder(game->instanceManager);
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
      .SetColourAttachmentFormat(VK_FORMAT_R8G8B8A8_SNORM) // FIXME: used to be VK_FORMAT_R16G16B16A16_SFLOAT, but
                                                           // that gives a validation error
      .SetDepthFormat(VK_FORMAT_D32_SFLOAT)
      .SetDepthCompareOperation(VK_COMPARE_OP_LESS_OR_EQUAL)
      .EnableBlending(Graphics::PipelineBuilder::BlendMode::ALPHA)
      .SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
      .Build();
}

template <> void AssetManager::DestroyAsset<Graphics::Pipeline *>(Graphics::Pipeline *&asset) const {
  Graphics::PipelineBuilder::DestroyPipeline(*asset, &game->instanceManager);
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

template <> std::string AssetManager::GetAssetPath<Graphics::Material *>(char const *assetName) const {
  return std::string("materials/") + assetName + ".mat";
}

template <>
AssetManager::AssetDSO<Graphics::Material *> *
AssetManager::ParseAsset<Graphics::Material *>(std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::Material *>();
  json<AssetDSO<Graphics::Material *>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <>
Graphics::Material *AssetManager::ConvertDSO<Graphics::Material *>(AssetDSO<Graphics::Material *> const *dso) {
  Graphics::Pipeline const *pipeline = LoadAsset<Graphics::Pipeline *>(dso->pipelineName);
  if (auto albedoAndBumpDSO = dynamic_cast<AlbedoAndBumpData *>(dso->instanceData)) {
    Graphics::Texture2D albedo = LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->albedoTexture);
    Graphics::Texture2D bump = LoadAsset<Graphics::Texture2D>(albedoAndBumpDSO->bumpTexture);
    return new Graphics::Materials::AlbedoAndBump(pipeline, albedo, bump, albedoAndBumpDSO->specularStrength,
                                                  albedoAndBumpDSO->phongExponent, albedoAndBumpDSO->hue);
  } else {
    ENGINE_ERROR("Unknown material type!");
    return nullptr;
  }
}

template <> void AssetManager::DestroyAsset<Graphics::Material *>(Graphics::Material *&asset) const { delete asset; }

} // namespace Engine

// JSON stuff

template <>
template <class TokenIterator>
inline constexpr TokenIterator json<Engine::AlbedoAndBumpData>::parse_tokenstream(TokenIterator begin,
                                                                                  TokenIterator end,
                                                                                  Engine::AlbedoAndBumpData &output) {
  if (begin->type == Token::Type::LBrace) {
    begin++;
    std::string key;
    bool is_last;
    do {
      begin = parse_key(begin, end, key);
      if (key == "albedoTexture") {
        begin = json<decltype(output.albedoTexture)>::parse_tokenstream(begin, end, output.albedoTexture);
      } else if (key == "bumpTexture") {
        begin = json<decltype(output.bumpTexture)>::parse_tokenstream(begin, end, output.bumpTexture);
      } else if (key == "hue") {
        begin = json<decltype(output.hue)>::parse_tokenstream(begin, end, output.hue);
      } else if (key == "specularStrength") {
        begin = json<decltype(output.specularStrength)>::parse_tokenstream(begin, end, output.specularStrength);
      } else if (key == "phongExponent") {
        begin = json<decltype(output.phongExponent)>::parse_tokenstream(begin, end, output.phongExponent);
      } else {
        throw std::runtime_error("Unexpected key in "
                                 "Engine::AlbedoAndBumpData"
                                 " : " +
                                 key);
      }
      begin = is_last_in_list(begin, end, is_last);
    } while (!is_last);
    return ++begin;
  }
  throw std::runtime_error("Expected left brace, got " + token_type_to_string(begin->type) + ".");
};

ABSTRACT_OBJECT_PARSER(Engine::MaterialInstanceData, ,
                       INHERITANCE_PARSER(Engine::MaterialInstanceData, Engine::AlbedoAndBumpData));

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Graphics::Material *>,
              FIELD_PARSER(pipelineName) FIELD_PARSER(instanceData) FIELD_PARSER(instanceData));