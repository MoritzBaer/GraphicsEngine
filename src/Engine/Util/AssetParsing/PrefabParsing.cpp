#include "AssetManager.h"

#include "Core/ECS.h"
#include "Editor/Display.h"
#include "Game.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"
#include "Members.h"
#include "json-parsing.h"

namespace Engine {

struct ComponentDSO {
  virtual void Dummy() {} // Dummy function to allow dynamic_cast
};

template <> struct AssetManager::AssetDSO<Core::Entity> {
  std::vector<ComponentDSO *> components;
};

struct TransformDSO : public ComponentDSO {
  Maths::Vector3 position;
  Maths::Quaternion rotation;
  Maths::Vector3 scale;
  std::vector<AssetManager::AssetDSO<Core::Entity>> children;
};

struct MeshRendererDSO : public ComponentDSO {
  std::string meshName;
  std::string materialName;
};

struct CameraDSO : public ComponentDSO {
  float fov;
  float nearClip;
  float farClip;
};

struct DisplayDSO : public ComponentDSO {
  std::string label;
};

template <> std::string AssetManager::AssetLoader<Core::Entity>::GetAssetPath(char const *assetName) const {
  return std::string("prefabs/") + assetName + ".pfb";
}

template <>
AssetManager::AssetDSO<Core::Entity> *
AssetManager::AssetLoader<Core::Entity>::ParseAsset(std::string const &assetSource) const {
  auto dso = new AssetDSO<Core::Entity>();
  json<AssetDSO<Core::Entity>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <> Core::Entity AssetManager::AssetLoader<Core::Entity>::ConvertDSO(AssetDSO<Core::Entity> const *dso) const {
  Core::Entity entity = members->ecs->CreateEntityAsPattern();
  for (auto component : dso->components) {
    if (auto transformDSO = dynamic_cast<TransformDSO *>(component)) {
      auto transform = entity.AddComponent<Graphics::Transform>();
      transform->position = transformDSO->position;
      transform->rotation = transformDSO->rotation;
      transform->scale = transformDSO->scale;
      for (auto &childDSO : transformDSO->children) {
        auto child = ConvertDSO(&childDSO);
        if (child.HasComponent<Graphics::Transform>()) {
          child.GetComponent<Graphics::Transform>()->SetParent(transform, false);
        }
      }
    } else if (auto meshRendererDSO = dynamic_cast<MeshRendererDSO *>(component)) {
      auto meshRenderer = entity.AddComponent<Graphics::MeshRenderer>();
      meshRenderer->mesh = members->assetManager->LoadAsset<Graphics::AllocatedMesh *>(meshRendererDSO->meshName);
      meshRenderer->material = members->assetManager->LoadAsset<Graphics::Material *>(meshRendererDSO->materialName);
    } else if (auto cameraDSO = dynamic_cast<CameraDSO *>(component)) {
      auto camera = entity.AddComponent<Graphics::Camera>();
      camera->projection =
          Maths::Transformations::Perspective(cameraDSO->nearClip, cameraDSO->farClip, cameraDSO->fov, 16.0f / 9.0f);
    } else if (auto displayDSO = dynamic_cast<DisplayDSO *>(component)) {
      entity.AddComponent<Editor::Display>()->AssignLabel(displayDSO->label.c_str());

    } else {
      ENGINE_ERROR("Unknown component type!");
    }
  }
  return entity;
}

template <> void AssetManager::AssetDestroyer<Core::Entity>::DestroyAsset(Core::Entity &asset) const {
  members->ecs->DestroyEntity(asset);
}

} // namespace Engine

OBJECT_PARSER(Engine::TransformDSO,
              FIELD_PARSER(position) FIELD_PARSER(rotation) FIELD_PARSER(scale) FIELD_PARSER(children));
OBJECT_PARSER(Engine::MeshRendererDSO, FIELD_PARSER(meshName) FIELD_PARSER(materialName));
OBJECT_PARSER(Engine::CameraDSO, FIELD_PARSER(fov) FIELD_PARSER(nearClip) FIELD_PARSER(farClip));
OBJECT_PARSER(Engine::DisplayDSO, FIELD_PARSER(label));

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Core::Entity>, FIELD_PARSER(components));

ABSTRACT_OBJECT_PARSER(Engine::ComponentDSO, ,
                       INHERITANCE_PARSER(Engine::ComponentDSO, Engine::TransformDSO)
                           INHERITANCE_PARSER(Engine::ComponentDSO, Engine::MeshRendererDSO)
                               INHERITANCE_PARSER(Engine::ComponentDSO, Engine::CameraDSO)
                                   INHERITANCE_PARSER(Engine::ComponentDSO, Engine::DisplayDSO));