#pragma once

#include "Core/ECS.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Util/Macros.h"
#include <string>
#include <unordered_map>

namespace Engine {
class AssetManager {
  _SINGLETON(AssetManager)

  std::unordered_map<std::string, Graphics::Shader> loadedShaders;
  std::unordered_map<std::string, Graphics::Pipeline *> loadedPipelines;
  std::unordered_map<std::string, Graphics::Material *> loadedMaterials;
  std::unordered_map<std::string, Graphics::Mesh> loadedMeshes;
  std::unordered_map<std::string, Graphics::Texture2D> loadedTextures;

public:
  static Graphics::Shader LoadShader(char const *shaderName, Graphics::ShaderType shaderType);
  inline static Graphics::Shader LoadShader(std::string const &shaderName, Graphics::ShaderType shaderType) {
    return LoadShader(shaderName.c_str(), shaderType);
  }
  static Graphics::Shader LoadShaderWithInferredType(char const *shaderName);
  inline static Graphics::Shader LoadShaderWithInferredType(std::string const &shaderName) {
    LoadShaderWithInferredType(shaderName.c_str());
  };

  static void InitStandins();

  static Graphics::Mesh LoadMeshFromOBJ(char const *meshName);
  static Graphics::AllocatedMesh *LoadMesh(char const *meshName, bool flipUVs = false);
  static Graphics::Material *LoadMaterial(char const *materialName);
  static Graphics::Pipeline const *LoadPipeline(char const *pipelineName);
  static Core::Entity LoadPrefab(char const *prefabName);
  static Graphics::Texture2D LoadTexture(char const *textureName);
};

} // namespace Engine
