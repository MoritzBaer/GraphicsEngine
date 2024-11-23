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
    : game(game), loadedShaders(), loadedPipelines(), loadedMaterials(), loadedMeshes(), loadedTextures() {}

Graphics::Shader AssetManager::LoadShader(char const *shaderName, Graphics::ShaderType shaderType) {
  PROFILE_FUNCTION()
  MAKE_FILE_PATH(shaderName, SHADER_PATH)
  _INSERT_ASSET_IF_NEW(shaderName, loadedShaders,
                       game->shaderCompiler.CompileShaderCode(shaderName, Util::FileIO::ReadFile(filePath), shaderType))
  Graphics::Shader &loadedShader = loadedShaders[shaderName];
  return loadedShader;
}

AssetManager::~AssetManager() {
  for (auto &texture : loadedTextures) {
    game->gpuObjectManager.DestroyTexture(texture.second);
  }

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder(game->instanceManager);
  for (auto &pipeline : loadedPipelines) {
    pipelineBuilder.DestroyPipeline(pipeline.second);
  }

  for (auto &shader : loadedShaders) {
    game->shaderCompiler.DestroyShader(shader.second);
  }
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
  Graphics::Mesh mesh = LoadMeshFromOBJ(meshName);
  if (flipUVs) {
    for (auto &vertex : mesh.vertices) {
      vertex.uv.y() = 1 - vertex.uv.y();
    }
  }
  return new Graphics::AllocatedMeshT(game->gpuObjectManager.AllocateMesh<Graphics::Vertex, Graphics::VertexFormat>(
      mesh)); // TODO: Decide if it would be better to store this in a map as well
}

Graphics::Pipeline *AssetManager::ParsePipeline(char const *pipelineData) {
  // Dummy implementation // TODO: implement properly
  Graphics::Shader vertexShader = LoadShaderWithInferredType("phong.vert");
  Graphics::Shader fragmentShader = LoadShaderWithInferredType("phong.frag");

  size_t uniformSize = sizeof(VkDeviceAddress) + sizeof(Matrix4);

  Graphics::PipelineBuilder pipelineBuilder = Graphics::PipelineBuilder(game->instanceManager);
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

Graphics::Material *ParseMAT(char const *materialname) {
  std::vector<char> materialData = Util::FileIO::ReadFile(materialname);
  return json<Graphics::Material *>::deserialize(materialData);
}

Graphics::Material *AssetManager::LoadMaterial(char const *materialName) {
  MAKE_FILE_PATH(materialName, MATERIAL_PATH);
  _RETURN_ASSET(materialName, loadedMaterials, ParseMAT(filePath));
}

Graphics::Pipeline const *AssetManager::LoadPipeline(char const *pipelineName) {
  _RETURN_ASSET(pipelineName, loadedPipelines, ParsePipeline(pipelineName));
}

Core::Entity AssetManager::LoadPrefab(char const *prefabName) {
  MAKE_FILE_PATH(prefabName, PREFAB_PATH);

  auto prefabData = Util::FileIO::ReadFile(filePath);
  Core::Entity e = game->ecs.CreateEntity();
  json<Core::Entity>::deserialize(prefabData, e);
  game->sceneHierarchy.BuildHierarchy();

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
    _RETURN_ASSET(textureName, loadedTextures,
                  game->gpuObjectManager.CreateTexture(Maths::Dimension2(width, height), pixels, VK_FILTER_LINEAR,
                                                       VK_FILTER_LINEAR))
  } else {
    return loadedTextures["missing"];
  }
}

} // namespace Engine

template <class Container>
Engine::Graphics::Material *json<Engine::Graphics::Material *>::deserialize(Container const &materialData,
                                                                            void *context) {
  using namespace Engine;

  auto pl = static_cast<Game *>(context)->assetManager.LoadPipeline("dummy");
  std::vector<Token> tokens{};
  tokenize(std::begin(materialData), std::end(materialData), std::back_inserter(tokens));

  auto tokenIt = tokens.begin();

  if (tokenIt++->type != Token::Type::LBrace) {
    ENGINE_ERROR("Expected '{{' at start of material data, got '{}'", token_type_to_string((tokenIt - 1)->type));
    return nullptr;
  }

  if (tokenIt->type != Token::Type::String) {
    ENGINE_ERROR("Expected identifier after '{{', got '{}'", token_type_to_string(tokenIt->type));
    return nullptr;
  }

  std::string materialType;
  tokenIt = parse_key(tokenIt, tokens.end(), materialType);
  Graphics::Material *mat = nullptr;

  if (materialType == "Engine::Graphics::Materials::AlbedoAndBump") {
    auto pl = static_cast<Game *>(context)->assetManager.LoadPipeline("dummy"); // TODO: Think of sensible system
    mat = new Graphics::Materials::AlbedoAndBump(pl, static_cast<Game *>(context)->assetManager.LoadTexture("white"),
                                                 static_cast<Game *>(context)->assetManager.LoadTexture("normalUp"));
    tokenIt = json<Graphics::Materials::AlbedoAndBump>::parse_tokenstream(
        tokenIt, tokens.end(), *dynamic_cast<Graphics::Materials::AlbedoAndBump *>(mat), context);
  } else {
    ENGINE_ERROR("Unknown material type '{}'", materialType);
    return nullptr;
  }

  if (tokenIt++->type != Token::Type::RBrace) {
    ENGINE_ERROR("Expected '}}' at end of material data, got '{}'", token_type_to_string((tokenIt - 1)->type));
    return nullptr;
  }

  return mat;
}
