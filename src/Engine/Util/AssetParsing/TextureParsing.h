#pragma once

#include "AssetManager.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/Texture.h"
#include "stb_image.h"

namespace Engine {

struct TextureDSO {
  int width, height, channels;
  stbi_uc *data;

  TextureDSO() : data(nullptr), width(0), height(0), channels(0) {}
  TextureDSO(TextureDSO &&other) {
    width = other.width;
    height = other.height;
    channels = other.channels;
    data = other.data;

    other.data = nullptr;
    other.width = other.height = 0;
  }
  ~TextureDSO() { free(data); }
  TextureDSO &operator=(TextureDSO &&other) {
    free(data);
    width = other.width;
    height = other.height;
    channels = other.channels;
    data = other.data;

    other.data = nullptr;
    other.width = other.height = 0;

    return *this;
  }
};

class TextureDestroyer {
  Graphics::GPUObjectManager RELEASE_CONST *gpuObjectManager;

public:
  TextureDestroyer(Graphics::GPUObjectManager RELEASE_CONST *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
  inline void DestroyAsset(Graphics::Texture2D &asset) const { gpuObjectManager->DestroyTexture(asset); }
};

class TextureCache {
  AssetCacheImpl<Graphics::Texture2D, TextureDestroyer> baseCache;

public:
  TextureCache(Graphics::GPUObjectManager RELEASE_CONST *gpuObjectManager);
  inline bool HasAsset(char const *assetName) const { return baseCache.HasAsset(assetName); }
  inline void InsertAsset(char const *assetName, Graphics::Texture2D const &asset) {
    baseCache.InsertAsset(assetName, asset);
  }
  inline Graphics::Texture2D LoadAsset(char const *assetName) const { return baseCache.LoadAsset(assetName); }
  inline void Clear() { baseCache.Clear(); }
};

class TextureParser {
public:
  TextureDSO ParseDSO(std::vector<char> const &source) const;
};

class TextureConverter {
  Graphics::GPUObjectManager RELEASE_CONST *gpuObjectManager;
  AssetManager *assetManager;

public:
  TextureConverter(Graphics::GPUObjectManager RELEASE_CONST *gpuObjectManager, AssetManager *assetManager)
      : gpuObjectManager(gpuObjectManager), assetManager(assetManager) {}
  Graphics::Texture2D ConvertDSO(TextureDSO const &dso) const;
};

using TextureLoader = AssetLoaderImpl<Graphics::Texture2D, TextureDSO, TextureParser, TextureConverter>;

} // namespace Engine