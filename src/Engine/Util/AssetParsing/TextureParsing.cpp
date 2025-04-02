#include "TextureParsing.h"

#include "Game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine {

template <> std::string AssetPath<Graphics::Texture2D>::FromName(char const *assetName) {
  return std::string("textures/") + assetName;
}

TextureCache::TextureCache(Graphics::GPUObjectManager *gpuObjectManager)
    : baseCache(TextureDestroyer(gpuObjectManager)) {
  uint32_t white = 0xFFFFFFFF;
  uint32_t normalUp = 0xFFFF8080;

  baseCache.InsertAsset("white", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &white));
  baseCache.InsertAsset("normalUp", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &normalUp));

  // Load error texture
  std::vector<uint32_t> errorTextureData(16 * 16, 0xFFFF00FF);
  for (int x = 0; x < 16; x++) {
    for (int y = x % 2; y < 16; y += 2) {
      errorTextureData[x * 16 + y] = 0xFF000000;
    }
  }
  baseCache.InsertAsset("missing", gpuObjectManager->CreateTexture(Maths::Dimension2(16, 16), errorTextureData.data(),
                                                                   VK_FILTER_NEAREST, VK_FILTER_NEAREST));
}

Graphics::Texture2D TextureConverter::ConvertDSO(TextureDSO const &dso) const {
  auto const *pixels = reinterpret_cast<uint32_t *>(dso.data);
  if (!pixels) {
    return assetManager->LoadAsset<Graphics::Texture2D>("missing");
  }
  return gpuObjectManager->CreateTexture(Maths::Dimension2(dso.width, dso.height), pixels, VK_FILTER_LINEAR,
                                         VK_FILTER_LINEAR);
}

TextureDSO TextureParser::ParseDSO(std::vector<char> const &source) const {
  auto dso = TextureDSO();
  dso.data = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(source.data()), source.size(), &dso.width,
                                   &dso.height, &dso.channels, STBI_rgb_alpha);
  return dso;
}

} // namespace Engine