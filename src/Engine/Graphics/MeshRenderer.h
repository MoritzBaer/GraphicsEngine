#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics {

struct MeshRenderer : public Core::ComponentT<MeshRenderer> {
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
      return this->mesh;
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
      return this->material;
    }
  } material;

  MeshRenderer(Core::Entity entity) : Core::ComponentT<MeshRenderer>(entity) {
    if (!entity.HasComponent<Transform>()) {
      entity.AddComponent<Transform>();
    }
  }

  ~MeshRenderer() {}

  inline void CopyFrom(MeshRenderer const &other) override {
    mesh = other.mesh;
    material = other.material;
  }
};

// Implementations
} // namespace Engine::Graphics