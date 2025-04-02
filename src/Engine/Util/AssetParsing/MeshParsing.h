#pragma once

#include "AssetManager.h"
#include "Graphics/AllocatedMesh.h"
#include "Graphics/GPUObjectManager.h"

namespace Engine {

struct MeshDSO {
  std::vector<Maths::Vector3> vertexPositions;
  std::vector<Maths::Vector3> vertexNormals;
  std::vector<Maths::Vector2> vertexUVs;
  std::vector<Maths::VectorT<3, Maths::VectorT<3, uint32_t>>> triangles;
};

struct MeshParser {
  MeshDSO ParseDSO(std::vector<char> const &source) const;
};

struct MeshConverter {
  Graphics::GPUObjectManager *gpuObjectManager;
  MeshConverter(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
  Graphics::AllocatedMesh *ConvertDSO(MeshDSO const &dso) const;
};

struct MeshDestroyer {
  Graphics::GPUObjectManager *gpuObjectManager;
  MeshDestroyer(Graphics::GPUObjectManager *gpuObjectManager) : gpuObjectManager(gpuObjectManager) {}
  void DestroyAsset(Graphics::AllocatedMesh *&asset) const;
};

using MeshLoader = AssetLoaderImpl<Graphics::AllocatedMesh *, MeshDSO, MeshParser, MeshConverter>;
using MeshCache = AssetCacheImpl<Graphics::AllocatedMesh *, MeshDestroyer>;

} // namespace Engine
