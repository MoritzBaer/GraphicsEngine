#pragma once

#include "Graphics/RenderingStrategies/ComputeBackground.h"
#include "MultiUseImplementations.h"

namespace Engine {

struct ShaderDSO {
  std::string shaderName;
  std::vector<char> shaderSource;
};

template <Graphics::ShaderType Type> class ShaderLoader {
  static inline std::string shaderDirPath = "res/shaders/";
  Graphics::ShaderCompiler *shaderCompiler;

  ShaderDSO FromFile(std::string const &fileName) const;
  std::string FileExtension() const;

public:
  ShaderLoader(Graphics::ShaderCompiler *shaderCompiler) : shaderCompiler(shaderCompiler) {}
  Graphics::Shader<Type> *ConvertDSO(ShaderDSO const &dso) const;
  Graphics::Shader<Type> *LoadAsset(const char *name) const;
};

class ShaderDestroyer {
  Graphics::ShaderCompiler const *shaderCompiler;

public:
  ShaderDestroyer(Graphics::ShaderCompiler const *shaderCompiler) : shaderCompiler(shaderCompiler) {}
  template <Graphics::ShaderType Type> void DestroyAsset(Graphics::Shader<Type> *&asset) const;
};

template <Graphics::ShaderType Type> using ShaderCache = AssetCacheImpl<Graphics::Shader<Type> *, ShaderDestroyer>;
template <Graphics::ShaderType Type>
using ShaderManager = TypeManagerImpl<Graphics::Shader<Type> *, ShaderLoader<Type>, ShaderCache<Type>>;

class CompiledEffectLoader {
  inline static constexpr VkPushConstantRange pushConstants{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(Engine::Graphics::RenderingStrategies::ComputePushConstants)};

  Graphics::DescriptorLayoutBuilder descriptorLayoutBuilder{nullptr};
  VkDescriptorSetLayout renderBufferDescriptorLayout;
  VkComputePipelineCreateInfo pipelineInfo;
  VkPipelineLayout computePipelineLayout;
  Graphics::InstanceManager const *instanceManager;
  AssetManager *assetManager;

public:
  CompiledEffectLoader(Graphics::InstanceManager const *instanceManager, AssetManager *assetManager)
      : instanceManager(instanceManager), assetManager(assetManager), descriptorLayoutBuilder(instanceManager),
        renderBufferDescriptorLayout(VK_NULL_HANDLE), pipelineInfo{}, computePipelineLayout(VK_NULL_HANDLE) {
    descriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    renderBufferDescriptorLayout = descriptorLayoutBuilder.Build(VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorLayoutBuilder.Clear();

    VkPipelineLayoutCreateInfo computeLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                 .setLayoutCount = 1,
                                                 .pSetLayouts = &renderBufferDescriptorLayout,
                                                 .pushConstantRangeCount = 1,
                                                 .pPushConstantRanges = &pushConstants};
    instanceManager->CreatePipelineLayout(&computeLayoutInfo, &computePipelineLayout);

    pipelineInfo = VkComputePipelineCreateInfo{.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                               .layout = computePipelineLayout};
  }

  CompiledEffectLoader(CompiledEffectLoader const &other)
      : CompiledEffectLoader(other.instanceManager, other.assetManager) {};

  ~CompiledEffectLoader() {
    instanceManager->DestroyDescriptorSetLayout(renderBufferDescriptorLayout);
    instanceManager->DestroyPipelineLayout(computePipelineLayout);
  }

  Graphics::RenderingStrategies::CompiledEffect LoadAsset(std::string const &effectName) const;
};

class CompiledEffectDestroyer {
  Graphics::InstanceManager const *instanceManager;

public:
  CompiledEffectDestroyer(Graphics::InstanceManager const *instanceManager) : instanceManager(instanceManager) {}
  void DestroyAsset(Graphics::RenderingStrategies::CompiledEffect &asset) const;
};

using CompiledEffectCache = AssetCacheImpl<Graphics::RenderingStrategies::CompiledEffect, CompiledEffectDestroyer>;

struct ComputeBackgroundDSO {
  std::string effectName;
  Graphics::RenderingStrategies::ComputePushConstants data;
};

class ComputeBackgroundConverter {
  Graphics::InstanceManager const *instanceManager;
  AssetManager *assetManager;

public:
  ComputeBackgroundConverter(Graphics::InstanceManager const *instanceManager, AssetManager *assetManager)
      : instanceManager(instanceManager), assetManager(assetManager) {}
  Graphics::RenderingStrategies::ComputeBackground *ConvertDSO(ComputeBackgroundDSO const &dso) const;
};

using ComputeBackgroundCache = DestroyerlessCacheImpl<Graphics::RenderingStrategies::ComputeBackground *>;
using ComputeBackgroundLoader =
    AssetLoaderImpl<Graphics::RenderingStrategies::ComputeBackground *, ComputeBackgroundDSO,
                    JsonParser<ComputeBackgroundDSO>, ComputeBackgroundConverter>;
using ComputeBackgroundManager = TypeManagerImpl<Graphics::RenderingStrategies::ComputeBackground *,
                                                 ComputeBackgroundLoader, ComputeBackgroundCache>;

template <Graphics::ShaderType Type>
Graphics::Shader<Type> *ShaderLoader<Type>::ConvertDSO(ShaderDSO const &dso) const {
  return shaderCompiler->CompileShaderCode<Type>(dso.shaderName.c_str(),
                                                 std::string(dso.shaderSource.begin(), dso.shaderSource.end()));
}

template <Graphics::ShaderType Type>
inline Graphics::Shader<Type> *ShaderLoader<Type>::LoadAsset(const char *name) const {
  std::string fileName = name + FileExtension();
  return ConvertDSO(FromFile(fileName));
}

template <Graphics::ShaderType Type> inline void ShaderDestroyer::DestroyAsset(Graphics::Shader<Type> *&asset) const {
  shaderCompiler->DestroyShader<Type>(asset);
}

template <Graphics::ShaderType Type> ShaderDSO ShaderLoader<Type>::FromFile(std::string const &fileName) const {
  auto filePath = ShaderLoader::shaderDirPath + fileName;
  return {.shaderName = fileName, .shaderSource = Util::FileIO::ReadFile(filePath.c_str())};
}

} // namespace Engine

JSON(Engine::ShaderDSO, FIELDS(shaderName))
JSON(Engine::ComputeBackgroundDSO, FIELDS(effectName, data))