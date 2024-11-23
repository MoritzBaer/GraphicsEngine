#pragma once

#include "Core/ECS.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Util/Macros.h"
#include <string>
#include <unordered_map>

struct Game;

namespace Engine {
class AssetManager {
  Game *game;
  std::unordered_map<std::string, Graphics::Shader> loadedShaders;
  std::unordered_map<std::string, Graphics::Pipeline *> loadedPipelines;
  std::unordered_map<std::string, Graphics::Material *> loadedMaterials;
  std::unordered_map<std::string, Graphics::Mesh> loadedMeshes;
  std::unordered_map<std::string, Graphics::Texture2D> loadedTextures;

  Graphics::Pipeline *ParsePipeline(char const *pipelineData);

public:
  AssetManager(Game *game);
  ~AssetManager();

  Graphics::Shader LoadShader(char const *shaderName, Graphics::ShaderType shaderType);
  inline Graphics::Shader LoadShader(std::string const &shaderName, Graphics::ShaderType shaderType) {
    return LoadShader(shaderName.c_str(), shaderType);
  }
  Graphics::Shader LoadShaderWithInferredType(char const *shaderName);
  inline Graphics::Shader LoadShaderWithInferredType(std::string const &shaderName) {
    LoadShaderWithInferredType(shaderName.c_str());
  };

  void InitStandins();

  Graphics::Mesh LoadMeshFromOBJ(char const *meshName);
  Graphics::AllocatedMesh *LoadMesh(char const *meshName, bool flipUVs = false);
  Graphics::Material *LoadMaterial(char const *materialName);
  Graphics::Pipeline const *LoadPipeline(char const *pipelineName);
  Core::Entity LoadPrefab(char const *prefabName);
  Graphics::Texture2D LoadTexture(char const *textureName);
};

} // namespace Engine
