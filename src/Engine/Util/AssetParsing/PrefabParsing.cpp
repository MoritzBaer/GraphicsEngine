#include "AssetManager.h"

#include "Core/ECS.h"
#include "Core/Scene.h"
#include "Editor/Display.h"
#include "Game.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"
#include "Members.h"
#include "json-parsing.h"

namespace Engine {

struct ComponentDSO {
  virtual ~ComponentDSO() = default;
};

struct EntityDSO {
  virtual ~EntityDSO() = default;
};

template <> struct AssetManager::AssetDSO<Core::Entity> : public EntityDSO {
  std::vector<ComponentDSO *> components;
};

struct TransformDSO : public ComponentDSO {
  Maths::Vector3 position;
  Maths::Quaternion rotation;
  Maths::Vector3 scale;
};

struct PrefabDSO : public EntityDSO {
  std::string prefabName;
  TransformDSO transform;
};

struct HierarchyDSO : public ComponentDSO {
  std::vector<EntityDSO *> children;
};

struct MeshRendererDSO : public ComponentDSO {
  std::string meshName;
  std::string materialName;
};

struct CameraDSO : public ComponentDSO {
  float fov;
  float nearClip;
  float farClip;
  float aspectRatio;
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

Core::Entity LoadPrefabDSO(PrefabDSO const *prefabDSO, AssetManager *assetManager, Core::ECS *targetECS) {
  auto transform = assetManager->LoadAsset<Core::Entity>(prefabDSO->prefabName)
                       .CopyToOtherECS(targetECS)
                       .GetComponent<Graphics::Transform>();

  transform->position = prefabDSO->transform.position;
  transform->rotation = prefabDSO->transform.rotation;
  transform->scale = prefabDSO->transform.scale;

  return transform->entity;
}

template <> Core::Entity AssetManager::AssetLoader<Core::Entity>::ConvertDSO(AssetDSO<Core::Entity> const *dso) const {
  Core::Entity entity = members->ecs->CreateEntity();
  for (auto component : dso->components) {
    if (auto transformDSO = dynamic_cast<TransformDSO *>(component)) {
      auto transform = entity.AddComponent<Graphics::Transform>();
      transform->position = transformDSO->position;
      transform->rotation = transformDSO->rotation;
      transform->scale = transformDSO->scale;
    } else if (auto meshRendererDSO = dynamic_cast<MeshRendererDSO *>(component)) {
      auto meshRenderer = entity.AddComponent<Graphics::MeshRenderer>();
      meshRenderer->mesh = members->assetManager->LoadAsset<Graphics::AllocatedMesh *>(meshRendererDSO->meshName);
      meshRenderer->material = members->assetManager->LoadAsset<Graphics::Material *>(meshRendererDSO->materialName);
    } else if (auto cameraDSO = dynamic_cast<CameraDSO *>(component)) {
      auto camera = entity.AddComponent<Graphics::Camera>();
      camera->projection = Maths::Transformations::Perspective(cameraDSO->nearClip, cameraDSO->farClip, cameraDSO->fov,
                                                               cameraDSO->aspectRatio);
    } else if (auto displayDSO = dynamic_cast<DisplayDSO *>(component)) {
      entity.AddComponent<Editor::Display>()->AssignLabel(displayDSO->label.c_str());
    } else if (auto hierarchyDSO = dynamic_cast<HierarchyDSO *>(component)) {
      auto hierarchy = entity.HasComponent<Core::HierarchyComponent>()
                           ? entity.GetComponent<Core::HierarchyComponent>()
                           : entity.AddComponent<Core::HierarchyComponent>();
      for (auto &childDSO : hierarchyDSO->children) {
        Core::Entity child{};
        if (auto prefabDSO = dynamic_cast<PrefabDSO *>(childDSO)) {
          child = LoadPrefabDSO(prefabDSO, members->assetManager, members->ecs);
        } else if (auto explicitDSO = dynamic_cast<AssetManager::AssetDSO<Core::Entity> *>(childDSO)) {
          child = ConvertDSO(explicitDSO);
        }
        if (!child.HasComponent<Core::HierarchyComponent>()) {
          child.AddComponent<Core::HierarchyComponent>();
        }
        auto childHierarchy = child.GetComponent<Core::HierarchyComponent>();
        hierarchy->children.push_back(childHierarchy);
        childHierarchy->parent = hierarchy;
      }

    } else {
      ENGINE_ERROR("Unknown component type!");
    }
  }
  return entity;
}

template <> void AssetManager::AssetDestroyer<Core::Entity>::DestroyAsset(Core::Entity &asset) const {
  members->ecs->DestroyEntity(asset);
}

// Scene

template <> struct AssetManager::AssetDSO<Core::Scene *> {
  std::vector<EntityDSO *> entities;
  std::string mainCamName;
};

template <> std::string AssetManager::AssetLoader<Core::Scene *>::GetAssetPath(char const *assetName) const {
  return std::string("scenes/") + assetName + ".scn";
}

template <>
AssetManager::AssetDSO<Core::Scene *> *
AssetManager::AssetLoader<Core::Scene *>::ParseAsset(std::string const &assetSource) const {
  auto dso = new AssetDSO<Core::Scene *>();
  json<AssetDSO<Core::Scene *>>::deserialize<std::string>(assetSource, *dso);
  return dso;
}

template <>
Core::Scene *AssetManager::AssetLoader<Core::Scene *>::ConvertDSO(AssetDSO<Core::Scene *> const *dso) const {
  Core::Scene *scene = new Core::Scene();
  AssetManager::AssetLoader<Core::Entity> entityLoader(&scene->ecs, members->assetManager);
  for (auto entityDSO : dso->entities) {
    if (auto explicitDSO = dynamic_cast<AssetManager::AssetDSO<Core::Entity> *>(entityDSO)) {
      entityLoader.ConvertDSO(explicitDSO);
    } else if (auto prefabDSO = dynamic_cast<PrefabDSO *>(entityDSO)) {
      LoadPrefabDSO(prefabDSO, members->assetManager, &scene->ecs);
    } else {
      ENGINE_ERROR("Unknown entity type encountered during scene parsing!");
    }
  }
  for (auto const &[display] : scene->ecs.FilterEntities<Editor::Display>()) {
    if (strcmp(display->label.data(), dso->mainCamName.c_str()) == 0) {
      scene->mainCamera = display->entity;
      break;
    }
  }
  scene->sceneHierarchy.Rebuild();
  return scene;
}

template <> void AssetManager::AssetDestroyer<Core::Scene *>::DestroyAsset(Core::Scene *&asset) const { delete asset; }

} // namespace Engine

OBJECT_PARSER(Engine::TransformDSO, FIELD_PARSER(position) FIELD_PARSER(rotation) FIELD_PARSER(scale));
OBJECT_PARSER(Engine::HierarchyDSO, FIELD_PARSER(children));
OBJECT_PARSER(Engine::MeshRendererDSO, FIELD_PARSER(meshName) FIELD_PARSER(materialName));
OBJECT_PARSER(Engine::CameraDSO,
              FIELD_PARSER(fov) FIELD_PARSER(nearClip) FIELD_PARSER(farClip) FIELD_PARSER(aspectRatio));
OBJECT_PARSER(Engine::DisplayDSO, FIELD_PARSER(label));

ABSTRACT_OBJECT_PARSER(Engine::ComponentDSO, ,
                       INHERITANCE_PARSER(Engine::ComponentDSO, Engine::TransformDSO)
                           INHERITANCE_PARSER(Engine::ComponentDSO, Engine::MeshRendererDSO)
                               INHERITANCE_PARSER(Engine::ComponentDSO, Engine::CameraDSO)
                                   INHERITANCE_PARSER(Engine::ComponentDSO, Engine::DisplayDSO)
                                       INHERITANCE_PARSER(Engine::ComponentDSO, Engine::HierarchyDSO));

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Core::Entity>, FIELD_PARSER(components));
OBJECT_PARSER(Engine::PrefabDSO, FIELD_PARSER(prefabName) FIELD_PARSER(transform));
ABSTRACT_OBJECT_PARSER(Engine::EntityDSO, ,
                       INHERITANCE_PARSER(Engine::EntityDSO, Engine::PrefabDSO)
                           INHERITANCE_PARSER(Engine::EntityDSO, Engine::AssetManager::AssetDSO<Engine::Core::Entity>));

OBJECT_PARSER(Engine::AssetManager::AssetDSO<Engine::Core::Scene *>, FIELD_PARSER(entities) FIELD_PARSER(mainCamName));