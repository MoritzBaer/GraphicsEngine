#include "ShaderParsing.h"

#include "AssetManager.h"
#include "Graphics/RenderingStrategies/ComputeBackground.h"

namespace Engine {

template <> std::string AssetPath<Graphics::RenderingStrategies::CompiledEffect>::FromName(char const *assetName) {
  return "rendering_strategies/background_compute_effects/" + std::string(assetName) + ".cef";
}

template <> std::string AssetPath<Graphics::RenderingStrategies::ComputeBackground *>::FromName(char const *assetName) {
  return "rendering_strategies/background_compute_effects/" + std::string(assetName) + ".cei";
}

template <> std::string ShaderLoader<Graphics::ShaderType::VERTEX>::FileExtension() const { return ".vert"; }
template <> std::string ShaderLoader<Graphics::ShaderType::GEOMETRY>::FileExtension() const { return ".geom"; }
template <> std::string ShaderLoader<Graphics::ShaderType::FRAGMENT>::FileExtension() const { return ".frag"; }
template <> std::string ShaderLoader<Graphics::ShaderType::COMPUTE>::FileExtension() const { return ".comp"; }

Graphics::RenderingStrategies::CompiledEffect CompiledEffectLoader::LoadAsset(std::string const &effectName) const {

  auto effectShader = assetManager->LoadAsset<Graphics::Shader<Graphics::ShaderType::COMPUTE>>(effectName);
  auto pipelineInfoCopy = pipelineInfo;
  pipelineInfoCopy.stage = effectShader.GetStageInfo();

  Graphics::RenderingStrategies::CompiledEffect effect{.pipelineLayout = computePipelineLayout,
                                                       .pipeline = VK_NULL_HANDLE};
  instanceManager->CreateComputePipeline(pipelineInfoCopy, &effect.pipeline);
  return effect;
}

void CompiledEffectDestroyer::DestroyAsset(Graphics::RenderingStrategies::CompiledEffect &asset) const {
  instanceManager->DestroyPipeline(asset.pipeline);
}

Graphics::RenderingStrategies::ComputeBackground *
ComputeBackgroundConverter::ConvertDSO(ComputeBackgroundDSO const &dso) const {
  auto effect = assetManager->LoadAsset<Graphics::RenderingStrategies::CompiledEffect>(dso.effectName);

  return new Graphics::RenderingStrategies::ComputeBackground(instanceManager, effect, dso.data);
}

} // namespace Engine