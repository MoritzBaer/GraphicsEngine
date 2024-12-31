#include "AssetManager.h"

#include "Game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine {

template <> struct AssetManager::AssetDSO<Graphics::Texture2D> {
  int width, height, channels;
  stbi_uc *data;
};

template <> std::string AssetManager::GetAssetPath<Graphics::Texture2D>(char const *assetName) const {
  return std::string("textures/") + assetName;
}

template <>
AssetManager::AssetDSO<Graphics::Texture2D> *
AssetManager::ParseAsset<Graphics::Texture2D>(std::string const &assetSource) const {
  auto dso = new AssetDSO<Graphics::Texture2D>();
  dso->data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(assetSource.data()), assetSource.size(),
                                    &dso->width, &dso->height, &dso->channels, STBI_rgb_alpha);
  return dso;
}

template <>
Graphics::Texture2D AssetManager::ConvertDSO<Graphics::Texture2D>(AssetDSO<Graphics::Texture2D> const *dso) {
  auto const *pixels =
      reinterpret_cast<uint32_t *>(dso->data); // TODO: Fix issues occurring when fewer channels are used
  if (!pixels) {
    return dynamic_cast<AssetCacheT<Graphics::Texture2D> *>(assetCaches[AssetTypeID<Graphics::Texture2D>::value])
        ->LoadAsset("missing");
  }
  return gpuObjectManager->CreateTexture(Maths::Dimension2(dso->width, dso->height), pixels, VK_FILTER_LINEAR,
                                         VK_FILTER_LINEAR);
}

template <> void AssetManager::DestroyAsset<Graphics::Texture2D>(Graphics::Texture2D &asset) const {
  gpuObjectManager->DestroyTexture(asset);
}

} // namespace Engine