#include "AssetManager.h"

#include "Core/ECS.h"
#include "Core/Scene.h"
#include "Core/Script.h"
#include "Game.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"
#include "Members.h"
#include "json-parsing.h"

#ifdef USER_COMPONENTS_SOURCE
#pragma message("USER_COMPONENTS_SOURCE defined as " USER_COMPONENTS_SOURCE)
#include USER_COMPONENTS_SOURCE
#endif

#ifdef USER_COMPONENTS
#define COMBINED_COMPONENTS ENGINE_COMPONENTS, USER_COMPONENTS
#else
#define COMBINED_COMPONENTS ENGINE_COMPONENTS
#endif

#ifdef USER_SCRIPTS_SOURCE
#pragma message("USER_SCRIPTS_SOURCE defined as " USER_SCRIPTS_SOURCE)
#include USER_SCRIPTS_SOURCE
#endif

#define ENGINE_COMPONENTS                                                                                              \
  Engine::TransformDSO, Engine::MeshRendererDSO, Engine::CameraDSO, Engine::HierarchyDSO, Engine::ScriptComponentDSO

namespace Engine {

struct EntityDSO {
  virtual ~EntityDSO() = default;
};

template <> struct AssetManager::AssetDSO<Core::Entity> : public EntityDSO {
  std::vector<ComponentDSO *> components;
};

struct TransformDSO : public ComponentDSO_T<Graphics::Transform> {
  Maths::Vector3 position;
  Maths::Quaternion rotation;
  Maths::Vector3 scale;

  void FillValues(Graphics::Transform *transform, AssetManager::LoaderMembers<Core::Entity> *loaderMembers) override {
    transform->position = position;
    transform->rotation = rotation;
    transform->scale = scale;
  }
};

struct PrefabDSO : public EntityDSO {
  std::string prefabName;
  TransformDSO transform;
};

struct HierarchyDSO : public ComponentDSO {
  std::vector<EntityDSO *> children;

  void AttachToEntity(Core::Entity entity, AssetManager::LoaderMembers<Core::Entity> *loaderMembers) override {
    throw std::runtime_error("HierarchyComponents are handled explicitly.");
  }
};

struct MeshRendererDSO : public ComponentDSO_T<Graphics::MeshRenderer> {
  std::string meshName;
  std::string materialName;

  void FillValues(Graphics::MeshRenderer *meshRenderer,
                  AssetManager::LoaderMembers<Core::Entity> *loaderMembers) override {
    meshRenderer->mesh = loaderMembers->assetManager->LoadAsset<Graphics::AllocatedMesh *>(meshName);
    meshRenderer->material = loaderMembers->assetManager->LoadAsset<Graphics::Material *>(materialName);
  }
};

struct CameraDSO : public ComponentDSO_T<Graphics::Camera> {
  float fov;
  float nearClip;
  float farClip;
  float aspectRatio;

  void FillValues(Graphics::Camera *camera, AssetManager::LoaderMembers<Core::Entity> *loaderMembers) override {
    camera->projection = Maths::Transformations::Perspective(nearClip, farClip, fov, aspectRatio);
  }
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
    if (auto hierarchyDSO = dynamic_cast<HierarchyDSO *>(component)) {
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
      component->AttachToEntity(entity, members);
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
  Core::EntityId mainCamId; // TODO: Replace by isViewportCamera-flag in CameraComponent (or similar)
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
  for (int i = 0; i < dso->entities.size(); i++) {
    auto entityDSO = dso->entities[i];
    Core::Entity entity;
    if (auto explicitDSO = dynamic_cast<AssetManager::AssetDSO<Core::Entity> *>(entityDSO)) {
      entity = entityLoader.ConvertDSO(explicitDSO);
    } else if (auto prefabDSO = dynamic_cast<PrefabDSO *>(entityDSO)) {
      entity = LoadPrefabDSO(prefabDSO, members->assetManager, &scene->ecs);
    } else {
      ENGINE_ERROR("Unknown entity type encountered during scene parsing!");
    }
    if (i == dso->mainCamId) {
      ENGINE_ASSERT(entity.HasComponent<Graphics::Camera>(), "Main camera must have a camera component!");
      scene->mainCamera = entity;
    }
  }
  scene->sceneHierarchy.Rebuild();
  return scene;
}

template <> void AssetManager::AssetDestroyer<Core::Scene *>::DestroyAsset(Core::Scene *&asset) const { delete asset; }

// Scripts

struct ScriptComponentDSO : public ComponentDSO_T<Core::ScriptComponent> {
  std::vector<ScriptDSO *> scripts;

  void FillValues(Core::ScriptComponent *scriptComponent,
                  AssetManager::LoaderMembers<Core::Entity> *loaderMembers) override {
    for (auto scriptDSO : scripts) {
      scriptDSO->Attach(scriptComponent, loaderMembers);
    }
  }
};

} // namespace Engine

JSON(Engine::TransformDSO, FIELDS(position, rotation, scale));
JSON(Engine::HierarchyDSO, FIELDS(children));
JSON(Engine::MeshRendererDSO, FIELDS(meshName, materialName));
JSON(Engine::CameraDSO, FIELDS(fov, nearClip, farClip, aspectRatio));
JSON(Engine::ScriptComponentDSO, FIELDS(scripts));

JSON(Engine::ComponentDSO *, SUBTYPES(COMBINED_COMPONENTS));

JSON(Engine::AssetManager::AssetDSO<Engine::Core::Entity>, FIELDS(components));
JSON(Engine::PrefabDSO, FIELDS(prefabName, transform));
JSON(Engine::EntityDSO *, SUBTYPES(Engine::PrefabDSO, Engine::AssetManager::AssetDSO<Engine::Core::Entity>));

JSON(Engine::AssetManager::AssetDSO<Engine::Core::Scene *>, FIELDS(entities, mainCamId));

#ifdef USER_SCRIPTS
JSON(Engine::ScriptDSO *, SUBTYPES(USER_SCRIPTS));
#endif