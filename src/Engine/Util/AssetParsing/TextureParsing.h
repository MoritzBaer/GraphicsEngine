#pragma once

#include "AssetManager.h"
#include "Graphics/GPUObjectManager.h"
#include "Graphics/Texture.h"
#include "stb_image.h"

namespace Engine {

struct TextureDSO {
  int width, height, channels;
  stbi_uc *data;
};

class TextureDestroyer {
  Graphics::GPUObjectManager *gpuObjectManager;

public:
  TextureDestroyer(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
  inline void DestroyAsset(Graphics::Texture2D &asset) const { gpuObjectManager->DestroyTexture(asset); }
};

class TextureCache {
  AssetCacheImpl<Graphics::Texture2D, TextureDestroyer> baseCache;

public:
  TextureCache(Graphics::GPUObjectManager *gpuObjectManager);
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
  Graphics::GPUObjectManager *gpuObjectManager;
  AssetManager *assetManager;

public:
  TextureConverter(Graphics::GPUObjectManager *gpuObjectManager, AssetManager *assetManager)
      : gpuObjectManager(gpuObjectManager), assetManager(assetManager) {}
  Graphics::Texture2D ConvertDSO(TextureDSO const &dso) const;
};

using TextureLoader = AssetLoaderImpl<Graphics::Texture2D, TextureDSO, TextureParser, TextureConverter>;

} // namespace Engine