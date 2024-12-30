#pragma once

#include "Core/ECS.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Util/FileIO.h"
#include "Util/Macros.h"
#include <string>
#include <unordered_map>

struct Game;

namespace Engine {

class AssetManager {
  uint8_t numberOfAssetTypes;
  Game *game;
  std::unordered_map<std::string, Graphics::Pipeline *> loadedPipelines;
  std::unordered_map<std::string, Graphics::Material *> loadedMaterials;
  std::unordered_map<std::string, Graphics::Mesh> loadedMeshes;
  std::unordered_map<std::string, Graphics::Texture2D> loadedTextures;
  std::unordered_map<std::string, Graphics::AllocatedMesh *> allocatedMeshes;

  Graphics::Pipeline *ParsePipeline(char const *pipelineData);

  typedef uint8_t typeID;
  template <typename T> struct AssetTypeID {
    inline static typeID id = -1;
  };

  class AssetCache {
  public:
    virtual void Clear() = 0;
  };

  std::array<AssetCache *, 255> assetCaches;

  template <typename T> class AssetCacheT : public AssetCache {
    std::unordered_map<std::string, T> cache;
    AssetManager const *manager;

  public:
    AssetCacheT(AssetManager const *manager) : manager(manager), cache() {}
    bool HasAsset(char const *assetName) { return cache.find(assetName) != cache.end(); }
    T LoadAsset(char const *assetName) { return cache[assetName]; }
    void InsertAsset(char const *assetName, T asset) { cache.insert({assetName, asset}); }

    void Clear() override {
      for (auto &pair : cache) {
        manager->DestroyAsset(pair.second);
      }
      cache.clear();
    }
  };

  template <typename T> void DestroyAsset(T &asset) const;

public:
  AssetManager(Game *game);
  ~AssetManager();

  template <typename T> struct AssetDSO;
  template <typename T> std::string GetAssetPath(char const *assetName) const;
  template <typename T> AssetDSO<T> *ParseAsset(std::string const &assetSource) const;
  template <typename T> T ConvertDSO(AssetDSO<T> const *dso);
  template <typename T> inline T LoadAsset(char const *assetName);
  template <typename T> inline T LoadAsset(std::string const &assetName) { return LoadAsset<T>(assetName.c_str()); };

  // Graphics::Shader LoadShader(char const *shaderName, Graphics::ShaderType shaderType);
  // inline Graphics::Shader LoadShader(std::string const &shaderName, Graphics::ShaderType shaderType) {
  //   return LoadShader(shaderName.c_str(), shaderType);
  // }
  // Graphics::Shader LoadShaderWithInferredType(char const *shaderName);
  // inline Graphics::Shader LoadShaderWithInferredType(std::string const &shaderName) {
  //   LoadShaderWithInferredType(shaderName.c_str());
  // };

  void InitStandins();

  Graphics::Mesh LoadMeshFromOBJ(char const *meshName);
  Graphics::AllocatedMesh *LoadMesh(char const *meshName, bool flipUVs = false);
  Graphics::Material *LoadMaterial(char const *materialName);
  Graphics::Pipeline const *LoadPipeline(char const *pipelineName);
  Core::Entity LoadPrefab(char const *prefabName);
  Graphics::Texture2D LoadTexture(char const *textureName);
};

template <typename T> inline T AssetManager::LoadAsset(char const *assetName) {
  if (AssetTypeID<T>::id == static_cast<typeID>(-1)) {
    AssetTypeID<T>::id = numberOfAssetTypes++;
  }
  if (!assetCaches[AssetTypeID<T>::id]) {
    assetCaches[AssetTypeID<T>::id] = new AssetCacheT<T>(this);
  }

  auto cache = dynamic_cast<AssetCacheT<T> *>(assetCaches[AssetTypeID<T>::id]);
  if (cache->HasAsset(assetName)) {
    return cache->LoadAsset(assetName);
  }

  auto const &assetPath = std::string("res/") + GetAssetPath<T>(assetName);
  auto const &assetSourceVec = Util::FileIO::ReadFile(assetPath);
  auto const &assetSource = std::string(assetSourceVec.begin(), assetSourceVec.end());
  auto dso = ParseAsset<T>(assetSource);
  T asset = ConvertDSO<T>(dso);
  free(dso);
  cache->InsertAsset(assetName, asset);
  return asset;
}

} // namespace Engine
