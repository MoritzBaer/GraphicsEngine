#include "PrefabParsing.h"

#include "Game.h"

namespace Engine {

template <> std::string AssetPath<Core::Entity>::FromName(char const *assetName) {
  return std::string("prefabs/") + assetName + ".pfb";
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

Core::Entity EntityConverter::ConvertDSO(ExplicitEntityDSO const &dso) const {
  Core::Entity entity = ecs->CreateEntity();
  for (auto component : dso.components) {
    if (auto hierarchyDSO = dynamic_cast<HierarchyDSO *>(component)) {
      auto hierarchy = entity.HasComponent<Core::HierarchyComponent>()
                           ? entity.GetComponent<Core::HierarchyComponent>()
                           : entity.AddComponent<Core::HierarchyComponent>();
      for (auto &childDSO : hierarchyDSO->children) {
        Core::Entity child{};
        if (auto prefabDSO = dynamic_cast<PrefabDSO *>(childDSO)) {
          child = LoadPrefabDSO(prefabDSO, assetManager, ecs);
        } else if (auto explicitDSO = dynamic_cast<ExplicitEntityDSO *>(childDSO)) {
          child = ConvertDSO(*explicitDSO);
        }
        if (!child.HasComponent<Core::HierarchyComponent>()) {
          child.AddComponent<Core::HierarchyComponent>();
        }
        auto childHierarchy = child.GetComponent<Core::HierarchyComponent>();
        hierarchy->children.push_back(childHierarchy);
        childHierarchy->parent = hierarchy;
      }
    } else {
      component->AttachToEntity(entity, assetManager);
    }
    delete component;
  }
  return entity;
}

// Scene

template <> std::string AssetPath<Core::Scene *>::FromName(char const *assetName) {
  return std::string("scenes/") + assetName + ".scn";
}

Core::Scene *SceneConverter::ConvertDSO(SceneDSO const &dso) const {
  Core::Scene *scene = new Core::Scene();
  EntityConverter entityConverter(assetManager, &scene->ecs);
  for (int i = 0; i < dso.entities.size(); i++) {
    auto entityDSO = dso.entities[i];
    Core::Entity entity;
    if (auto explicitDSO = dynamic_cast<ExplicitEntityDSO *>(entityDSO)) {
      entity = entityConverter.ConvertDSO(*explicitDSO);
    } else if (auto prefabDSO = dynamic_cast<PrefabDSO *>(entityDSO)) {
      entity = LoadPrefabDSO(prefabDSO, assetManager, &scene->ecs);
    } else {
      ENGINE_ERROR("Unknown entity type encountered during scene parsing!");
    }

    delete entityDSO;

    if (i == dso.mainCamId) {
      ENGINE_ASSERT(entity.HasComponent<Graphics::Camera>(), "Main camera must have a camera component!");
      scene->mainCamera = entity;
    }
  }
  scene->sceneHierarchy.Rebuild();
  return scene;
}

// Scripts

Core::Scene *SceneCache::LoadAsset(char const *assetName) const {
  auto pattern = baseCache.LoadAsset(assetName);
  auto copy = new Core::Scene();
  copy->ecs.Copy(&pattern->ecs);
  copy->sceneHierarchy.Rebuild();
  copy->mainCamera = pattern->mainCamera.InOtherECS(&copy->ecs);
  for (auto &[transform] : copy->ecs.FilterEntities<Graphics::Transform>()) {
    if (transform->hierarchy->parent) {
      transform->parent = transform->hierarchy->parent->entity.GetComponent<Graphics::Transform>();
    }
  }
  return copy;
}

Core::Entity EntityManager::LoadAsset(char const *assetName) {
  if (cache.HasAsset(assetName)) {
    return cache.LoadAsset(assetName);
  }
  auto entity = loader.LoadAsset(assetName);
  cache.InsertAsset(assetName, entity);
  return entity;
}

} // namespace Engine