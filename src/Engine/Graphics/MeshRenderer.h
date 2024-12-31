#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics {

ENGINE_COMPONENT_DECLARATION(MeshRenderer) {
private:
public:
  struct {
  private:
    AllocatedMesh *mesh = nullptr;

  public:
    inline operator AllocatedMesh *() { return mesh; }
    inline operator AllocatedMesh const *() const { return mesh; }
    inline AllocatedMesh *&operator=(AllocatedMesh *mesh) {
      // Maybe make copy of the mesh. In that case, delete the old mesh at assignment
      this->mesh = mesh;
      return mesh;
    }
  } mesh;
  struct {
  private:
    Material *material = nullptr;

  public:
    inline operator Material *() { return material; }
    inline operator Material const *() const { return material; }
    inline Material *&operator=(Material *material) {
      // TODO: Eventually use copy of given material
      this->material = material;
      return material;
    }
  } material;

  ENGINE_COMPONENT_CONSTRUCTOR(MeshRenderer) {
    if (!entity.HasComponent<Transform>()) {
      entity.AddComponent<Transform>();
    }
  }

  ~MeshRenderer() {}

  inline void CopyFrom(Core::Component const *other) override {
    if (auto otherMeshRenderer = dynamic_cast<MeshRenderer const *>(other)) {
      mesh = otherMeshRenderer->mesh;
      material = otherMeshRenderer->material;
    } else {
      ENGINE_ERROR("Tried to copy MeshRenderer from different type!");
    }
  }
};

// Implementations
} // namespace Engine::Graphics