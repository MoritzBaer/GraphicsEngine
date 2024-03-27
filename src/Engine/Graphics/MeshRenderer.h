#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics {

ENGINE_COMPONENT_DECLARATION(MeshRenderer) {
  AllocatedMesh *mesh;
  Material *material;

  ENGINE_COMPONENT_CONSTRUCTOR(MeshRenderer), material(nullptr), mesh(nullptr) {
    if (!entity.HasComponent<Transform>()) {
      entity.AddComponent<Transform>();
    }
  }

  template <typename T_CPU, typename T_GPU> void SetMesh(MeshT<T_CPU, T_GPU> const &mesh);

  ~MeshRenderer() { delete mesh; }
};

// Implementations
template <typename T_CPU, typename T_GPU> void MeshRenderer::SetMesh(MeshT<T_CPU, T_GPU> const &mesh) {
  if (this->mesh) {
    delete this->mesh;
  }
  this->mesh = new AllocatedMeshT(mesh);
}
} // namespace Engine::Graphics
