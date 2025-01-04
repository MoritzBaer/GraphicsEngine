#include "AssetManager.h"

#include "Game.h"

namespace Engine {
AssetManager::AssetManager(Graphics::GPUObjectManager *gpuObjectManager, Core::ECS *ecs,
                           Graphics::ShaderCompiler *shaderCompiler, Graphics::InstanceManager *instanceManager)
    : ecs(ecs), gpuObjectManager(gpuObjectManager), shaderCompiler(shaderCompiler), instanceManager(instanceManager),
      typeManagers() {
  InitStandins();
}

AssetManager::~AssetManager() {
  for (int c = 0; c < nextFreeType; c++) {
    delete typeManagers[c];
  }
}

void AssetManager::InitStandins() {
  // TODO: Move to cache implementation probably
  // auto textureManager =
  //     dynamic_cast<SingleTypeManagerT<Graphics::Texture2D> *>(typeManagers[AssetTypeID<Graphics::Texture2D>::value]);
  //
  // uint32_t white = 0xFFFFFFFF;
  // uint32_t normalUp = 0xFFFF8080;
  // textureManager->InsertAsset("white", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &white));
  // textureManager->InsertAsset("normalUp", gpuObjectManager->CreateTexture(Maths::Dimension2(1, 1), &normalUp));

  // Load error texture
}

} // namespace Engine
