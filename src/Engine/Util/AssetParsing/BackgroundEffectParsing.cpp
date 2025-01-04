#include "AssetManager.h"

#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "Members.h"

namespace Engine {

template <> struct AssetManager::AssetDSO<Graphics::RenderingStrategies::CompiledEffect> {
  std::string shaderName;
};

template <>
AssetManager::AssetDSO<Graphics::RenderingStrategies::CompiledEffect> *
AssetManager::AssetLoader<Graphics::RenderingStrategies::CompiledEffect>::ParseAsset(
    std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::RenderingStrategies::CompiledEffect>();
  json<AssetDSO<Graphics::RenderingStrategies::CompiledEffect>>::deserialize(assetSource, *dso);
  return dso;
}

template <>
std::string
AssetManager::AssetLoader<Graphics::RenderingStrategies::CompiledEffect>::GetAssetPath(char const *assetName) const {
  return "rendering_strategies/background_compute_effects/" + std::string(assetName) + ".cef";
}

template <>
Graphics::RenderingStrategies::CompiledEffect
AssetManager::AssetLoader<Graphics::RenderingStrategies::CompiledEffect>::ConvertDSO(
    AssetDSO<Graphics::RenderingStrategies::CompiledEffect> const *dso) const {

  auto effectShader =
      members->assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::COMPUTE>>(dso->shaderName.c_str());

  members->pipelineInfo.stage = effectShader.GetStageInfo();

  Graphics::RenderingStrategies::CompiledEffect effect{.pipelineLayout = members->computePipelineLayout,
                                                       .pipeline = nullptr};
  members->instanceManager->CreateComputePipeline(members->pipelineInfo, &effect.pipeline);
  return effect;
}

template <>
void AssetManager::AssetDestroyer<Graphics::RenderingStrategies::CompiledEffect>::DestroyAsset(
    Graphics::RenderingStrategies::CompiledEffect &asset) const {
  members->instanceManager->DestroyPipeline(asset.pipeline);
}

// Effect instance
template <>
std::string AssetManager::AssetLoader<Graphics::RenderingStrategies::ComputeBackground *>::GetAssetPath(
    char const *assetName) const {
  return "rendering_strategies/background_compute_effects/" + std::string(assetName) + ".cei";
}

template <> struct AssetManager::AssetDSO<Graphics::RenderingStrategies::ComputeBackground *> {
  std::string effectName;
  Graphics::RenderingStrategies::ComputePushConstants data;
};

template <>
AssetManager::AssetDSO<Graphics::RenderingStrategies::ComputeBackground *> *
AssetManager::AssetLoader<Graphics::RenderingStrategies::ComputeBackground *>::ParseAsset(
    std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::RenderingStrategies::ComputeBackground *>();
  json<AssetDSO<Graphics::RenderingStrategies::ComputeBackground *>>::deserialize(assetSource, *dso);
  return dso;
}

template <>
Graphics::RenderingStrategies::ComputeBackground *
AssetManager::AssetLoader<Graphics::RenderingStrategies::ComputeBackground *>::ConvertDSO(
    AssetDSO<Graphics::RenderingStrategies::ComputeBackground *> const *dso) const {
  auto effect =
      members->assetManager->LoadAsset<Graphics::RenderingStrategies::CompiledEffect>(dso->effectName.c_str());

  return new Graphics::RenderingStrategies::ComputeBackground(members->instanceManager, effect, dso->data);
}

template <>
void AssetManager::AssetDestroyer<Graphics::RenderingStrategies::ComputeBackground *>::DestroyAsset(
    Graphics::RenderingStrategies::ComputeBackground *&asset) const {
  delete asset;
}

} // namespace Engine

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Graphics::RenderingStrategies::CompiledEffect>,
              FIELD_PARSER(shaderName))
OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Graphics::RenderingStrategies::ComputeBackground *>,
              FIELD_PARSER(effectName) FIELD_PARSER(data))