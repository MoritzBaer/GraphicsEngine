#pragma once

#include "Debug/Logging.h"
#include "Util/FileIO.h"
#include "Util/Macros.h"
#include <array>
#include <string>
#include <unordered_map>

// +----------+
// | CONCEPTS |
// +----------+

template <typename T_Parser, class T_Container, typename T_DSO>
concept DSOParser = requires(T_Parser const &parser, T_Container const &assetSource) {
  { parser.ParseDSO(assetSource) } -> std::convertible_to<T_DSO>;
};

template <typename T_Loader, typename T_Asset>
concept AssetLoader = requires(T_Loader const &loader, char const *assetName) {
  { loader.LoadAsset(assetName) } -> std::convertible_to<T_Asset>;
};

template <typename T_Converter, typename T_DSO, typename T_Asset>
concept DSOConverter = requires(T_Converter const &converter, T_DSO const &dso) {
  { converter.ConvertDSO(dso) } -> std::convertible_to<T_Asset>;
};

template <typename T_Destroyer, typename T_Asset>
concept AssetDestroyer = requires(T_Destroyer const &destroyer, T_Asset &asset) {
  { destroyer.DestroyAsset(asset) } -> std::same_as<void>;
};

template <typename T_Cache, typename T_Asset>
concept AssetCache = requires(T_Cache const &constCache, T_Cache &cache, char const *assetName, T_Asset const &asset) {
  { cache.InsertAsset(assetName, asset) } -> std::same_as<void>;
  { constCache.HasAsset(assetName) } -> std::convertible_to<bool>;
  { constCache.LoadAsset(assetName) } -> std::convertible_to<T_Asset>;
  { cache.Clear() } -> std::same_as<void>;
};

template <typename T_Manager, typename T_Asset>
concept TypeManager = requires(T_Manager &manager, char const *assetName) {
  { manager.LoadAsset(assetName) } -> std::convertible_to<T_Asset>;
  { manager.Cleanup() } -> std::same_as<void>;
};

namespace Engine {

template <typename T_Asset> struct AssetPath {
  static std::string FromName(char const *assetName);
};

// +-------------------------+
// | DEFAULT IMPLEMENTATIONS |
// +-------------------------+

template <typename T_Asset, AssetDestroyer<T_Asset> T_Destroyer> class AssetCacheImpl {
  std::unordered_map<std::string, T_Asset> cache;
  T_Destroyer destroyer;

public:
  template <typename... DestroyerArgs> AssetCacheImpl(DestroyerArgs... args) : cache(), destroyer(args...) {}
  inline void InsertAsset(char const *assetName, T_Asset const &asset) { cache.insert({assetName, asset}); }
  inline bool HasAsset(char const *assetName) const { return cache.find(assetName) != cache.end(); }
  inline T_Asset LoadAsset(char const *assetName) const { return cache.find(assetName)->second; }
  inline void Clear() {
    for (auto &[_, asset] : cache) {
      destroyer.DestroyAsset(asset);
    }
    cache.clear();
  }
  inline auto begin() { return cache.begin(); }
  inline auto end() { return cache.end(); }
};

template <typename T_Asset> class DestroyerlessCacheImpl {
  std::unordered_map<std::string, T_Asset> cache;

public:
  template <typename... DestroyerArgs> DestroyerlessCacheImpl(DestroyerArgs... args) : cache() {}
  inline void InsertAsset(char const *assetName, T_Asset const &asset) { cache.insert({assetName, asset}); }
  inline bool HasAsset(char const *assetName) const { return cache.find(assetName) != cache.end(); }
  inline T_Asset LoadAsset(char const *assetName) const { return cache.find(assetName)->second; }
  inline void Clear() { cache.clear(); }
  inline auto begin() { return cache.begin(); }
  inline auto end() { return cache.end(); }
};

template <typename T_Asset, typename T_DSO, DSOParser<std::vector<char>, T_DSO> T_Parser,
          DSOConverter<T_DSO, T_Asset> T_Converter>
class AssetLoaderImpl {
  T_Parser parser;
  T_Converter converter;

public:
  AssetLoaderImpl(T_Parser parser, T_Converter converter) : parser(parser), converter(converter) {}
  template <typename... ConverterArgs> AssetLoaderImpl(ConverterArgs... args) : parser(), converter(args...) {}

  inline T_Asset LoadAsset(char const *assetName) const {
    auto const &assetPath = "res/" + AssetPath<T_Asset>::FromName(assetName);
    auto const &assetSource = Util::FileIO::ReadFile(assetPath);
    auto const &dso = parser.ParseDSO(assetSource);
    return converter.ConvertDSO(dso);
  }
};

template <typename T_Asset, AssetLoader<T_Asset> T_Loader, AssetCache<T_Asset> T_Cache> class TypeManagerImpl {
  T_Loader loader;
  T_Cache cache;

public:
  TypeManagerImpl(T_Loader const &loader, T_Cache const &cache) : loader(loader), cache(cache) {}

  inline void Cleanup() { cache.Clear(); }

  inline T_Asset LoadAsset(char const *assetName) {
    if (!cache.HasAsset(assetName)) {
      cache.InsertAsset(assetName, loader.LoadAsset(assetName));
    }
    return cache.LoadAsset(assetName);
  }
};

// +-------------------------+
// | ASSET MANAGER INTERFACE |
// +-------------------------+

class AssetManager {

  class ListableManager {
  public:
    virtual void Cleanup() = 0;
  };

  template <typename T_Asset> class ListableTypeManager : public ListableManager {
  public:
    virtual T_Asset LoadAsset(char const *assetName) = 0;
  };

  template <typename T_Asset, TypeManager<T_Asset> T_ManagerImpl>
  class ListableTypeManagerImpl : public ListableTypeManager<T_Asset> {
    T_ManagerImpl manager;
    friend class AssetManager;

  public:
    ListableTypeManagerImpl(T_ManagerImpl const &manager) : manager(manager) {}
    template <typename... ManagerArgs> ListableTypeManagerImpl(ManagerArgs... args) : manager(args...) {}
    T_Asset LoadAsset(char const *assetName) override { return manager.LoadAsset(assetName); }
    void Cleanup() override { manager.Cleanup(); }
  };

  typedef uint8_t typeID;
  template <typename T_Asset> struct AssetTypeID {
    inline static typeID value = -1;
  };
  inline static typeID nextFreeType = 0;

  std::array<ListableManager *, std::numeric_limits<typeID>::max()> typeManagers;

public:
  AssetManager() : typeManagers(nullptr) {}
  ~AssetManager() {
    for (int c = 0; c < nextFreeType; c++) {
      if (typeManagers[c])
        delete typeManagers[c];
    }
  }

  template <typename T_Asset> inline bool IsRegistered() const {
    return AssetTypeID<T_Asset>::value != typeID(-1) && typeManagers[AssetTypeID<T_Asset>::value];
  }

  template <typename T_Asset, TypeManager<T_Asset> T_Manager>
  inline T_Manager *RegisterAssetType(T_Manager const &manager) {
    if (AssetTypeID<T_Asset>::value == typeID(-1)) {
      AssetTypeID<T_Asset>::value = nextFreeType++;
    }
    auto listableManager = typeManagers[AssetTypeID<T_Asset>::value] =
        new ListableTypeManagerImpl<T_Asset, T_Manager>(manager);
    return &dynamic_cast<ListableTypeManagerImpl<T_Asset, T_Manager> *>(listableManager)->manager;
  }

  template <typename T_Asset, AssetLoader<T_Asset> T_Loader, AssetCache<T_Asset> T_Cache>
  inline TypeManagerImpl<T_Asset, T_Loader, T_Cache> *RegisterAssetType(T_Loader const &loader, T_Cache const &cache) {
    return RegisterAssetType<T_Asset, TypeManagerImpl<T_Asset, T_Loader, T_Cache>>(loader, cache);
  }

  template <typename T_Asset, TypeManager<T_Asset> T_Manager, typename... ManagerArgs>
  inline T_Manager *RegisterAssetType(ManagerArgs... args) {
    return RegisterAssetType<T_Asset, T_Manager>(T_Manager(args...));
  }

  template <typename T_Asset> inline T_Asset LoadAsset(char const *assetName) {
    if (AssetTypeID<T_Asset>::value == typeID(-1)) {
      ENGINE_ERROR("Tried to load unregistered asset type {}!", typeid(T_Asset).name());
      return T_Asset();
    }
    auto manager = dynamic_cast<ListableTypeManager<T_Asset> *>(typeManagers[AssetTypeID<T_Asset>::value]);
    if (!manager) {
      ENGINE_ERROR("No manager for registered asset type {}!", typeid(T_Asset).name());
      return T_Asset();
    }
    return manager->LoadAsset(assetName);
  }
  template <typename T_Asset> inline T_Asset LoadAsset(std::string const &assetName) {
    return LoadAsset<T_Asset>(assetName.c_str());
  }
};

#if 0

class AssetManager {

  typedef uint8_t typeID;
  template <typename T> struct AssetTypeID {
    inline static typeID value = -1;
  };
  inline static typeID nextFreeType = 0;

public:
  template <typename T> struct AssetDSO;

  template <typename T> struct DestroyerMembers;
  template <typename T> struct LoaderMembers;

  template <typename T> class AssetCache {
  public:
    virtual ~AssetCache() = default;
    virtual void InsertAsset(char const *assetName, T const &asset) = 0;
    virtual bool HasAsset(char const *assetName) = 0;
    virtual T LoadAsset(char const *assetName) = 0;
    virtual void Clear() = 0;
  };

  template <typename T> class AssetLoader {
    template <typename T_Other> friend class AssetLoader;
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
  public:
    virtual ~SingleTypeManager() = default;
  };

  template <typename T> class SingleTypeManagerT : public SingleTypeManager {
    AssetCache<T> *cache;
    AssetLoader<T> *loader;

    friend class AssetManager;
    inline void InsertAsset(char const *assetName, T const &asset) { cache->InsertAsset(assetName, asset); }

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
  AssetManager() : typeManagers() {};
  ~AssetManager() {
    for (int c = 0; c < nextFreeType; c++) {
      if (typeManagers[c])
        delete typeManagers[c];
    }
  };

  template <typename T> inline void RegisterAssetType(AssetCache<T> *cache, LoaderMembers<T> *loaderMembers);
  template <typename T>
  inline void RegisterAssetType(SingleTypeManagerT<T>) template <typename T, typename... LoaderArgs>
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
  ~AssetCacheImpl() override {
    for (auto &asset : cache) {
      destroyer.DestroyAsset(asset.second);
    }
  }
  inline bool HasAsset(char const *assetName) override { return cache.find(assetName) != cache.end(); }
  T LoadAsset(char const *assetName) override;
  inline void InsertAsset(char const *assetName, T const &asset) override { cache.insert({assetName, asset}); }
  inline void Clear() override { cache.clear(); }
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

#endif

} // namespace Engine
