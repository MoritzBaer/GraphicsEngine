#include "AssetManager.h"

#include "Core/SceneHierarchy.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/TestMaterial.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"

#include <functional>
#include <stdlib.h>
#include <unordered_map>

#define RESOURCE_PATH "res/"
#define SHADER_PATH "shaders/"
#define MESH_PATH "meshes/"
#define MATERIAL_PATH "materials/"
#define PREFAB_PATH "prefabs/"

#define CONSTRUCT_PATHS(a, b) a b

#define MAKE_FILE_PATH(fileName, directoryName)                                                                        \
  char dirPath[] = CONSTRUCT_PATHS(RESOURCE_PATH, directoryName);                                                      \
  char *filePath = static_cast<char *>(malloc(strlen(fileName) * sizeof(char) + sizeof(dirPath)));                     \
  strcpy(filePath, dirPath);                                                                                           \
  strcat(filePath, fileName);

#define _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                  \
  bool isNew = instance->table.find(key) == instance->table.end();                                                     \
  if (isNew) {                                                                                                         \
    instance->table.insert({key, constructor});                                                                        \
  }

#define _RETURN_ASSET(key, table, constructor)                                                                         \
  _INSERT_ASSET_IF_NEW(key, table, constructor)                                                                        \
  return instance->table[key];

namespace Engine {

AssetManager::AssetManager()
    : loadedShaders(), loadedMaterials(), loadedMeshes(), loadedPipelines(), loadedTextures() {}
AssetManager::~AssetManager() {}
void AssetManager::Init() { instance = new AssetManager(); }
void AssetManager::Cleanup() { delete instance; }

Graphics::Shader AssetManager::LoadShader(char const *shaderName, Graphics::ShaderType shaderType) {
  PROFILE_FUNCTION()
  MAKE_FILE_PATH(shaderName, SHADER_PATH)
  _INSERT_ASSET_IF_NEW(shaderName, loadedShaders,
                       Graphics::ShaderCompiler::CompileShaderCode(Util::FileIO::ReadFile(filePath), shaderType))
  Graphics::Shader &loadedShader = instance->loadedShaders[shaderName];
  if (isNew) {
    mainDeletionQueue.Push(&loadedShader);
  }
  return loadedShader;
}

Graphics::Shader AssetManager::LoadShaderWithInferredType(char const *shaderName) {
  char const *extension = strrchr(shaderName, '.');

  if (strcmp(extension, ".vert") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::VERTEX);
  } else if (strcmp(extension, ".frag") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::FRAGMENT);
  } else if (strcmp(extension, ".comp") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::COMPUTE);
  } else if (strcmp(extension, ".geom") == 0) {
    return LoadShader(shaderName, Graphics::ShaderType::GEOMETRY);
  }

  ENGINE_ERROR("Could not infer shader type from file extension: {}!", extension);
  return Graphics::Shader();
}

void AssetManager::InitStandins() {

  // Load error texture
  std::vector<uint32_t> errorTextureData(16 * 16, 0xFF00FFFF);
  for (int x = 0; x < 16; x++) {
    for (int y = x % 2; y < 16; y += 2) {
      errorTextureData[x * 16 + y] = 0x000000FF;
    }
  }
  instance->loadedTextures.insert(
      {"missing", Graphics::Texture2D({16, 16}, errorTextureData.data(), VK_FILTER_NEAREST, VK_FILTER_NEAREST)});
}

Graphics::Mesh AssetManager::LoadMeshFromOBJ(char const *meshName) {
  MAKE_FILE_PATH(meshName, MESH_PATH)
  auto meshData = Util::FileIO::ReadFile(filePath);
  _RETURN_ASSET(meshName, loadedMeshes, Util::ParseOBJ(meshData.data()))
}

Graphics::AllocatedMesh *AssetManager::LoadMesh(char const *meshName) {
  Graphics::Mesh mesh = LoadMeshFromOBJ(meshName);
  return new Graphics::AllocatedMeshT(mesh); // TODO: Decide if it would be better to store this in a map as well
}

Graphics::Pipeline *ParsePipeline(char const *pipelineData) {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader vertexShader = AssetManager::LoadShader("coloured_triangle_mesh.vert", Graphics::ShaderType::VERTEX);
  Graphics::Shader fragmentShader = AssetManager::LoadShader("texture.frag", Graphics::ShaderType::FRAGMENT);

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Maths::Matrix4) + sizeof(Maths::Vector3);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder();
  return pipelineBuilder.AddPushConstant(uniformSize, 0, Graphics::ShaderType::VERTEX)
      .AddDescriptorBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Graphics::ShaderType::FRAGMENT)
      .SetShaderStages(vertexShader, fragmentShader)
      .SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .SetPolygonMode(VK_POLYGON_MODE_FILL)
      .SetColourAttachmentFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
      .SetDepthFormat(VK_FORMAT_D32_SFLOAT)
      .SetDepthCompareOperation(VK_COMPARE_OP_LESS_OR_EQUAL)
      .EnableBlending(Graphics::PipelineBuilder::BlendMode::ALPHA)
      .Build();
}

Graphics::Material *ParseMAT(char const *materialData) {
  auto pl = AssetManager::LoadPipeline("dummy");
  return new Graphics::TestMaterial(pl, Maths::Vector3{0.3, 0.9, 0.5}, AssetManager::LoadTexture("dummy"));
}

Graphics::Material *AssetManager::LoadMaterial(char const *materialName) {
  MAKE_FILE_PATH(materialName, MATERIAL_PATH)
  // auto materialData = Util::FileIO::ReadFile(filePath);
  char const *materialData = "dummy";
  //_INSERT_ASSET_IF_NEW(materialName, loadedMaterials, ParseMAT(materialData.data()))
  bool isNew = instance->loadedMaterials.find(materialName) == instance->loadedMaterials.end();
  if (isNew) {
    instance->loadedMaterials.insert({materialName, ParseMAT(materialData)});
  }
  Graphics::Material const *loadedMaterial = instance->loadedMaterials[materialName];
  return new Graphics::TestMaterial(loadedMaterial);
}

Graphics::Pipeline const *AssetManager::LoadPipeline(char const *pipelineName) {
  _INSERT_ASSET_IF_NEW(pipelineName, loadedPipelines, ParsePipeline(pipelineName));
  auto p = instance->loadedPipelines[pipelineName];
  if (isNew) {
    mainDeletionQueue.Push(p);
  }
  return p;
}

Core::Entity AssetManager::LoadPrefab(char const *prefabName) {
  MAKE_FILE_PATH(prefabName, PREFAB_PATH);

  auto prefabData = Util::FileIO::ReadFile(filePath);
  const char *dataString = prefabData.data();
  auto e = Util::ParseEntity(dataString);
  Core::SceneHierarchy::BuildHierarchy();
  return e;
}

Graphics::Texture2D AssetManager::LoadTexture(char const *textureName) { return instance->loadedTextures["missing"]; }

} // namespace Engine