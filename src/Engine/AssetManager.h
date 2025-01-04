#pragma once

#include "Core/ECS.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/Shader.h"
#include "Util/FileIO.h"
#include "Util/Macros.h"
#include <string>
#include <unordered_map>

namespace Engine {

class AssetManager {

  typedef uint8_t typeID;
  template <typename T> struct AssetTypeID {
    inline static typeID value = -1;
  };
  inline static typeID nextFreeType = 0;
  Core::ECS *ecs;
  Graphics::GPUObjectManager *gpuObjectManager;
  Graphics::ShaderCompiler *shaderCompiler;
  Graphics::InstanceManager *instanceManager;

public:
  template <typename T> struct AssetDSO;

  template <typename T> struct DestroyerMembers;
  template <typename T> struct LoaderMembers;

  template <typename T> class AssetCache {

  public:
    virtual void InsertAsset(char const *assetName, T asset) = 0;
    virtual bool HasAsset(char const *assetName) = 0;
    virtual T LoadAsset(char const *assetName) = 0;
  };

  template <typename T> class AssetLoader {
    LoaderMembers<T> *members;
    std::string GetAssetPath(char const *assetName) const;
    AssetDSO<T> *ParseAsset(std::string const &assetSource) const;
    T ConvertDSO(AssetDSO<T> const *dso) const;

  public:
    AssetLoader(LoaderMembers<T> *members) : members(members) {};
    template <typename... Args> AssetLoader(Args... args) : members(new LoaderMembers<T>(args...)) {}
    ~AssetLoader() { delete members; }
    inline T LoadAsset(char const *assetName) const;
  };

  template <typename T> class AssetDestroyer {
    DestroyerMembers<T> *members;

  public:
    AssetDestroyer(DestroyerMembers<T> *members) : members(members) {}
    template <typename... Args> AssetDestroyer(Args... args) : members(new DestroyerMembers<T>(args...)) {}
    ~AssetDestroyer() { delete members; }
    void DestroyAsset(T &asset) const;
  };

private:
  class SingleTypeManager {
    virtual void Dummy() {} // Dummy function to allow dynamic_cast
  };
  template <typename T> class SingleTypeManagerT : public SingleTypeManager {
    AssetCache<T> *cache;
    AssetLoader<T> *loader;

    friend class AssetManager;
    inline void InsertAsset(char const *assetName, T asset) { cache->InsertAsset(assetName, asset); }

  public:
    inline SingleTypeManagerT(AssetCache<T> *cache, AssetLoader<T> *loader) : cache(cache), loader(loader) {}
    inline ~SingleTypeManagerT() {
      delete cache;
      delete loader;
    }
    inline T LoadAsset(char const *assetName) {
      if (!cache->HasAsset(assetName)) {
        cache->InsertAsset(assetName, loader->LoadAsset(assetName));
      }
      return cache->LoadAsset(assetName);
    }
  };

  std::array<SingleTypeManager *, 255> typeManagers;

public:
  AssetManager(Graphics::GPUObjectManager *gpuObjectManager, Core::ECS *ecs, Graphics::ShaderCompiler *shaderCompiler,
               Graphics::InstanceManager *instanceManager);
  ~AssetManager();

  void InitStandins();
  template <typename T> inline void RegisterAssetType(AssetCache<T> *cache, LoaderMembers<T> *loaderMembers);
  template <typename T, typename... LoaderArgs>
  inline void RegisterAssetType(AssetCache<T> *cache, LoaderArgs... args) {
    RegisterAssetType<T>(cache, new LoaderMembers<T>(args...));
  }
  template <typename T> inline T LoadAsset(char const *assetName);
  template <typename T> inline T LoadAsset(std::string const &assetName) { return LoadAsset<T>(assetName.c_str()); };
};

// IMPLEMENTATIONS

template <typename T> inline T AssetManager::AssetLoader<T>::LoadAsset(char const *assetName) const {
  auto const &assetPath = "res/" + GetAssetPath(assetName);
  auto const &assetSourceVec = Util::FileIO::ReadFile(assetPath);
  auto const &assetSource = std::string(assetSourceVec.begin(), assetSourceVec.end());
  auto dso = ParseAsset(assetSource);
  T asset = ConvertDSO(dso);
  free(dso);
  return asset;
}

template <typename T> class AssetCacheImpl : public AssetManager::AssetCache<T> {
  std::unordered_map<std::string, T> cache;
  AssetManager::AssetDestroyer<T> destroyer;

public:
  AssetCacheImpl(AssetManager::AssetDestroyer<T> const &destroyer) : destroyer(destroyer), cache() {}
  template <typename... Args> AssetCacheImpl(Args... args) : destroyer(args...), cache() {}
  ~AssetCacheImpl() {
    for (auto &asset : cache) {
      destroyer.DestroyAsset(asset.second);
    }
  }
  inline bool HasAsset(char const *assetName) { return cache.find(assetName) != cache.end(); }
  T LoadAsset(char const *assetName);
  inline void InsertAsset(char const *assetName, T asset) { cache.insert({assetName, asset}); }
};

template <typename T> inline T AssetCacheImpl<T>::LoadAsset(char const *assetName) {
  auto cachedAsset = cache.find(assetName);
  return cachedAsset->second;
}

template <typename T> inline void AssetManager::RegisterAssetType(AssetCache<T> *cache, LoaderMembers<T> *members) {
  if (AssetTypeID<T>::value == typeID(-1)) {
    AssetTypeID<T>::value = nextFreeType++;
  }
  typeManagers[AssetTypeID<T>::value] = new SingleTypeManagerT<T>(cache, new AssetLoader<T>(members));
}

template <typename T> inline T AssetManager::LoadAsset(char const *assetName) {
  if (AssetTypeID<T>::value == typeID(-1)) {
    ENGINE_ERROR("No manager for unregistered asset type!");
    return T();
  }
  auto manager = dynamic_cast<SingleTypeManagerT<T> *>(typeManagers[AssetTypeID<T>::value]);
  if (!manager) {
    ENGINE_ERROR("No manager for registered asset type!");
    return T();
  }
  return manager->LoadAsset(assetName);
}

} // namespace Engine
