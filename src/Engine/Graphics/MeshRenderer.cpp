#include "MeshRenderer.h"

#include "AssetManager.h"

void Engine::Graphics::MeshRenderer::AssignMesh(std::string meshName) {
  baseMesh = meshName;
  SetMesh(AssetManager::LoadMeshFromOBJ(baseMesh.c_str()));
}

void Engine::Graphics::MeshRenderer::AssignMaterial(std::string materialName) {
  baseMaterial = materialName;
  material = AssetManager::LoadMaterial(baseMaterial.c_str());
}
