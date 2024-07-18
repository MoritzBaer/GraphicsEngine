#pragma once

#include "AllocatedMesh.h"
#include "Core/ECS.h"
#include "Material.h"
#include "Transform.h"

namespace Engine::Graphics {

ENGINE_COMPONENT_DECLARATION(MeshRenderer) {
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

  void AssignMesh(std::string meshName);
  void AssignMaterial(std::string materialName);
  template <typename T_CPU, typename T_GPU> void SetMesh(MeshT<T_CPU, T_GPU> const &mesh);

  ~MeshRenderer() { delete mesh; }

  std::string const &GetBaseMesh() const { return baseMesh; }
  std::string const &GetBaseMaterial() const { return baseMaterial; }
  struct {
    MeshRenderer &parent;
    void operator=(std::string const &value) { parent.AssignMesh(value.c_str()); }
  } baseMeshAccessor = {*this};
  struct {
    MeshRenderer &parent;
    void operator=(std::string const &value) { parent.AssignMaterial(value.c_str()); }
  } baseMaterialAccessor = {*this};
};

// Implementations
template <typename T_CPU, typename T_GPU> void MeshRenderer::SetMesh(MeshT<T_CPU, T_GPU> const &mesh) {
  if (this->mesh) {
    delete this->mesh;
  }
  this->mesh = new AllocatedMeshT(mesh);
}
} // namespace Engine::Graphics

OBJECT_PARSER(
    Engine::Graphics::MeshRenderer,
    if (key == "baseMesh") {
      std::string value;
      begin = json<std::string>::parse_tokenstream(begin, end, value, context);
      output.AssignMesh(value);
    } else if (key == "baseMaterial") {
      std::string value;
      begin = json<std::string>::parse_tokenstream(begin, end, value, context);
      output.AssignMaterial(value);
    } else)
OBJECT_SERIALIZER(Engine::Graphics::MeshRenderer, output = json<const char *>::serialize("baseMesh", output);
                  *output++ = ':'; *output++ = ' '; output = json<std::string>::serialize(object.GetBaseMesh(), output);
                  output = json<const char *>::serialize("baseMaterial", output); *output++ = ':'; *output++ = ' ';
                  output = json<std::string>::serialize(object.GetBaseMaterial(), output) *output++ = ',';)
