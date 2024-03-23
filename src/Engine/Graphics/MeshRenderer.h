#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(MeshRenderer) {
  AllocatedMesh mesh;
  Material *material;

  ENGINE_COMPONENT_CONSTRUCTOR(MeshRenderer) {
    if (!entity.HasComponent<Transform>()) {
      entity.AddComponent<Transform>();
    }
  }

  ~MeshRenderer() { mesh.Destroy(); }
};
} // namespace Engine::Graphics
