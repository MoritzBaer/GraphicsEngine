#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"
#include "Util/Serializable.h"

namespace Engine::Graphics {

ENGINE_COMPONENT_DECLARATION(MeshRenderer), public Util::Serializable {
private:
  std::string baseMesh;
  std::string baseMaterial;

public:
  AllocatedMesh *mesh;
  Material *material;

  ENGINE_COMPONENT_CONSTRUCTOR(MeshRenderer), material(nullptr), mesh(nullptr), baseMesh(""), baseMaterial("") {
    if (!entity.HasComponent<Transform>()) {
      entity.AddComponent<Transform>();
    }
  }

  void AssignMesh(const char *meshName);
  void AssignMaterial(const char *materialName);
  template <typename T_CPU, typename T_GPU> void SetMesh(MeshT<T_CPU, T_GPU> const &mesh);

  ~MeshRenderer() { delete mesh; }

  void Serialize(std::stringstream & targetStream) const override;
};

// Implementations
template <typename T_CPU, typename T_GPU> void MeshRenderer::SetMesh(MeshT<T_CPU, T_GPU> const &mesh) {
  if (this->mesh) {
    delete this->mesh;
  }
  this->mesh = new AllocatedMeshT(mesh);
}
} // namespace Engine::Graphics
