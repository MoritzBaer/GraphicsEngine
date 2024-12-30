#include "AssetManager.h"

#include "Core/SceneHierarchy.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "Graphics/MeshRenderer.h"
#include "Serialization/Entity.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"
#define STB_IMAGE_IMPLEMENTATION
#include "Game.h"
#include "stb_image.h"

#include <functional>
#include <stdlib.h>
#include <unordered_map>

#define RESOURCE_PATH "res/"
#define SHADER_PATH "shaders/"
#define MESH_PATH "meshes/"
#define MATERIAL_PATH "materials/"
#define PREFAB_PATH "prefabs/"
#define TEXTURE_PATH "textures/"

#define CONSTRUCT_PATHS(a, b) a b

#define MAKE_FILE_PATH(fileName, directoryName)                                                                        \
  char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, directoryName);                                                      \
  char *filePath = static_cast<char *>(malloc(strlen(fileName) * sizeof(char) + sizeof(dirPath)));                     \
  strcpy(filePath, dirPath);                                                                                           \
  strcat(filePath, fileName);

#define _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                  \
  bool isNew = table.find(key) == table.end();                                                                         \
  if (isNew) {                                                                                                         \
    table.insert({key, constructor});                                                                                  \
  }

#define _RETURN_ASSET(key, table, constructor)                                                                         \
  _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                        \
  return table[key];

template <> struct json<Engine::Graphics::Material *> {
  template <typename T_Other> friend struct json;

  template <class Container>
  static Engine::Graphics::Material *deserialize(Container const &json, void *context = nullptr);
  template <class OutputIterator>
  static constexpr OutputIterator serialize(Engine::Graphics::Material const &object, OutputIterator output);
};

namespace Engine {
AssetManager::AssetManager(Game *game)
    : game(game), loadedPipelines(), loadedMaterials(), loadedMeshes(), loadedTextures(), numberOfAssetTypes(0),
      assetCaches() {}

AssetManager::~AssetManager() {
  for (int c = 0; c < numberOfAssetTypes; c++) {
    assetCaches[c]->Clear();
    delete assetCaches[c];
  }
  for (auto &texture : loadedTextures) {
    game->gpuObjectManager.DestroyTexture(texture.second);
  }

  for (auto &mesh : allocatedMeshes) {
    game->gpuObjectManager.DeallocateMesh(mesh.second);
  }
}

void AssetManager::InitStandins() {
  uint32_t white = 0xFFFFFFFF;
  uint32_t normalUp = 0xFFFF8080;
  loadedTextures.insert({"white", game->gpuObjectManager.CreateTexture(Maths::Dimension2(1, 1), &white)});
  loadedTextures.insert({"normalUp", game->gpuObjectManager.CreateTexture(Maths::Dimension2(1, 1), &normalUp)});

  // Load error texture
  std::vector<uint32_t> errorTextureData(16 * 16, 0xFFFF00FF);
  for (int x = 0; x < 16; x++) {
    for (int y = x % 2; y < 16; y += 2) {
      errorTextureData[x * 16 + y] = 0xFF000000;
    }
  }
  loadedTextures.insert(
      {"missing", game->gpuObjectManager.CreateTexture(Maths::Dimension2(16, 16), errorTextureData.data(),
                                                       VK_FILTER_NEAREST, VK_FILTER_NEAREST)});
}

Graphics::Mesh AssetManager::LoadMeshFromOBJ(char const *meshName) {
  MAKE_FILE_PATH(meshName, MESH_PATH)
  auto meshData = Util::FileIO::ReadFile(filePath);
  _RETURN_ASSET(meshName, loadedMeshes, Util::ParseOBJ(meshData.data()))
}

Graphics::AllocatedMesh *AssetManager::LoadMesh(char const *meshName, bool flipUVs) {
  return LoadAsset<Graphics::AllocatedMesh *>(meshName);
}

Graphics::Material *AssetManager::LoadMaterial(char const *materialName) {
  return LoadAsset<Graphics::Material *>(materialName);
}

Graphics::Texture2D AssetManager::LoadTexture(char const *textureName) {
  return LoadAsset<Graphics::Texture2D>(textureName);
}

Core::Entity AssetManager::LoadPrefab(char const *prefabName) {
  return LoadAsset<Core::Entity>(prefabName);
  MAKE_FILE_PATH(prefabName, PREFAB_PATH);

  auto prefabData = Util::FileIO::ReadFile(filePath);
  Core::Entity e = game->ecs.CreateEntity();
  json<Core::Entity>::deserialize(prefabData, e);
  game->sceneHierarchy.BuildHierarchy();

  return e;
}

} // namespace Engine
