#include "MeshRenderer.h"

#include "AssetManager.h"

void Engine::Graphics::MeshRenderer::AssignMesh(const char *meshName) {
  baseMesh = meshName;
  SetMesh(AssetManager::LoadMeshFromOBJ(baseMesh));
}

void Engine::Graphics::MeshRenderer::AssignMaterial(const char *materialName) {
  baseMaterial = materialName;
  material = AssetManager::LoadMaterial(baseMaterial);
}

void Engine::Graphics::MeshRenderer::Serialize(std::stringstream &targetStream) const {
  targetStream << "MeshRenderer: { mesh: " << baseMesh << ", material: " << baseMaterial << " }";
}
