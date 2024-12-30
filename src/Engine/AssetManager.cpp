#include "AssetManager.h"

#include "Game.h"

namespace Engine {
AssetManager::AssetManager(Game *game) : game(game), numberOfAssetTypes(0), assetCaches() {}

AssetManager::~AssetManager() {
  for (int c = 0; c < numberOfAssetTypes; c++) {
    assetCaches[c]->Clear();
    delete assetCaches[c];
  }
}

void AssetManager::InitStandins() {
  InitCacheIfNecessary<Graphics::Texture2D>();

  auto textureCache =
      dynamic_cast<AssetCacheT<Graphics::Texture2D> *>(assetCaches[AssetTypeID<Graphics::Texture2D>::id]);

  uint32_t white = 0xFFFFFFFF;
  uint32_t normalUp = 0xFFFF8080;
  textureCache->InsertAsset("white", game->gpuObjectManager.CreateTexture(Maths::Dimension2(1, 1), &white));
  textureCache->InsertAsset("normalUp", game->gpuObjectManager.CreateTexture(Maths::Dimension2(1, 1), &normalUp));

  // Load error texture
  std::vector<uint32_t> errorTextureData(16 * 16, 0xFFFF00FF);
  for (int x = 0; x < 16; x++) {
    for (int y = x % 2; y < 16; y += 2) {
      errorTextureData[x * 16 + y] = 0xFF000000;
    }
  }
  textureCache->InsertAsset("missing",
                            game->gpuObjectManager.CreateTexture(Maths::Dimension2(16, 16), errorTextureData.data(),
                                                                 VK_FILTER_NEAREST, VK_FILTER_NEAREST));
}

} // namespace Engine
