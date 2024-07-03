#include "AssetManager.h"

#include "Core/SceneHierarchy.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/Materials/AlbedoAndBump.h"
#include "Graphics/Materials/TestMaterial.h"
#include "Graphics/MeshRenderer.h"
#include "Util/FileIO.h"
#include "Util/Parsing.h"
#define STB_IMAGE_IMPLEMENTATION
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
  _INSERT_ASSET_IF_NEW(
      shaderName, loadedShaders,
      Graphics::ShaderCompiler::CompileShaderCode(shaderName, Util::FileIO::ReadFile(filePath), shaderType))
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
  std::vector<uint32_t> errorTextureData(16 * 16, 0xFFFF00FF);
  for (int x = 0; x < 16; x++) {
    for (int y = x % 2; y < 16; y += 2) {
      errorTextureData[x * 16 + y] = 0xFF000000;
    }
  }
  instance->loadedTextures.insert(
      {"missing", Graphics::Texture2D({16, 16}, errorTextureData.data(), VK_FILTER_NEAREST, VK_FILTER_NEAREST)});
  mainDeletionQueue.Push(&instance->loadedTextures["missing"]);
}

Graphics::Mesh AssetManager::LoadMeshFromOBJ(char const *meshName) {
  MAKE_FILE_PATH(meshName, MESH_PATH)
  auto meshData = Util::FileIO::ReadFile(filePath);
  _RETURN_ASSET(meshName, loadedMeshes, Util::ParseOBJ(meshData.data()))
}

Graphics::AllocatedMesh *AssetManager::LoadMesh(char const *meshName, bool flipUVs) {
  Graphics::Mesh mesh = LoadMeshFromOBJ(meshName);
  if (flipUVs) {
    for (auto &vertex : mesh.vertices) {
      vertex.uv.y() = 1 - vertex.uv.y();
    }
  }
  return new Graphics::AllocatedMeshT(mesh); // TODO: Decide if it would be better to store this in a map as well
}

Graphics::Pipeline *ParsePipeline(char const *pipelineData) {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader vertexShader = AssetManager::LoadShaderWithInferredType("phong.vert");
  Graphics::Shader fragmentShader = AssetManager::LoadShaderWithInferredType("phong.frag");

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Matrix4);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder();
  return pipelineBuilder.AddPushConstant(uniformSize, 0, Graphics::ShaderType::VERTEX)
      .AddDescriptorBinding(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      .AddDescriptorBinding(1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      .AddDescriptorBinding(1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      .SetShaderStages(vertexShader, fragmentShader)
      .BindSetInShaders(0, Graphics::ShaderType::VERTEX, Graphics::ShaderType::FRAGMENT)
      .BindSetInShaders(1, Graphics::ShaderType::FRAGMENT)
      .SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .SetPolygonMode(VK_POLYGON_MODE_FILL)
      .SetColourAttachmentFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
      .SetDepthFormat(VK_FORMAT_D32_SFLOAT)
      .SetDepthCompareOperation(VK_COMPARE_OP_LESS_OR_EQUAL)
      .EnableBlending(Graphics::PipelineBuilder::BlendMode::ALPHA)
      .SetCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
      .Build();
}

Graphics::Material *ParseMAT(char const *materialData) {
  auto pl = AssetManager::LoadPipeline("dummy");
  return new Graphics::Materials::AlbedoAndBump(pl, AssetManager::LoadTexture("spaceship_texture.png"),
                                                AssetManager::LoadTexture("spaceship_normals.png"));
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
  return new Graphics::Materials::AlbedoAndBump(loadedMaterial);
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

Graphics::Texture2D AssetManager::LoadTexture(char const *textureName) {
  MAKE_FILE_PATH(textureName, TEXTURE_PATH)
  // TODO: Only load file if texture is not already loaded

  int width, height, channels;
  stbi_uc *pixelChannels = stbi_load(filePath, &width, &height, &channels, STBI_rgb_alpha);
  uint32_t *pixels =
      reinterpret_cast<uint32_t *>(pixelChannels); // TODO: Fix issues occurring when fewer channels are used

  if (pixels) {
    _INSERT_ASSET_IF_NEW(textureName, loadedTextures,
                         Graphics::Texture2D({width, height}, pixels, VK_FILTER_LINEAR, VK_FILTER_LINEAR))
    auto &t = instance->loadedTextures[textureName];
    if (isNew) {
      mainDeletionQueue.Push(t);
    }
    return t;
  } else {
    return instance->loadedTextures["missing"];
  }
}

} // namespace Engine