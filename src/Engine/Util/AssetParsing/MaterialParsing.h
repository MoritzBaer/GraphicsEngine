#pragma once

#include "AssetManager.h"
#include "Graphics/Material.h"
#include "MultiUseImplementations.h"

namespace Engine {
struct PipelineDSO {
  std::string vertexShaderName;
  std::string fragmentShaderName;
  std::string geometryShaderName;
};

class PipelineConverter {
  AssetManager *assetManager;
  Graphics::InstanceManager const *instanceManager;

public:
  PipelineConverter(AssetManager *assetManager, Graphics::InstanceManager const *instanceManager)
      : assetManager(assetManager), instanceManager(instanceManager) {}
  Graphics::Pipeline *ConvertDSO(PipelineDSO const &dso) const;
};

class PipelineDestroyer {
  Graphics::InstanceManager const *instanceManager;

public:
  PipelineDestroyer(Graphics::InstanceManager const *instanceManager) : instanceManager(instanceManager) {}
  void DestroyAsset(Graphics::Pipeline *&asset) const;
};

using PipelineLoader = AssetLoaderImpl<Graphics::Pipeline *, PipelineDSO, JsonParser<PipelineDSO>, PipelineConverter>;
using PipelineCache = AssetCacheImpl<Graphics::Pipeline *, PipelineDestroyer>;
using PipelineManager = TypeManagerImpl<Graphics::Pipeline *, PipelineLoader, PipelineCache>;

struct MaterialInstanceData {
  virtual ~MaterialInstanceData() = default;
};

struct MaterialDSO {
  std::string pipelineName;
  MaterialInstanceData *instanceData;
};

class MaterialConverter {
  AssetManager *assetManager;

public:
  MaterialConverter(AssetManager *assetManager) : assetManager(assetManager) {}
  Graphics::Material *ConvertDSO(MaterialDSO const &dso) const;
};

using MaterialLoader = AssetLoaderImpl<Graphics::Material *, MaterialDSO, JsonParser<MaterialDSO>, MaterialConverter>;
using MaterialCache = AssetCacheImpl<Graphics::Material *, OwnedDestroyer<Graphics::Material>>;
using MaterialManager = TypeManagerImpl<Graphics::Material *, MaterialLoader, MaterialCache>;

struct AlbedoAndBumpData : public MaterialInstanceData {
  std::string albedoTexture;
  std::string bumpTexture;
  Maths::Vector3 hue{1.0f, 1.0f, 1.0f};
  float specularStrength; // TODO: Extract into Phong
  float phongExponent;
};

} // namespace Engine

JSON(Engine::PipelineDSO, FIELDS(vertexShaderName, fragmentShaderName, geometryShaderName));

JSON(Engine::MaterialDSO, FIELDS(pipelineName, instanceData));

JSON(Engine::AlbedoAndBumpData, FIELDS(albedoTexture, bumpTexture, hue, specularStrength, phongExponent));

JSON(Engine::MaterialInstanceData *, SUBTYPES(Engine::AlbedoAndBumpData));