#pragma once

#include "AssetManager.h"
#include "ComponentParsing.h"
#include "Core/Scene.h"
#include "Core/Script.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"
#include "MultiUseImplementations.h"
#include "ScriptParsing.h"
#include "json-parsing.h"

#ifdef USER_SCRIPTS_SOURCE
#pragma message("USER_SCRIPTS_SOURCE defined as " USER_SCRIPTS_SOURCE)
#include USER_SCRIPTS_SOURCE
#endif

#define ENGINE_COMPONENTS                                                                                              \
  Engine::TransformDSO, Engine::MeshRendererDSO, Engine::CameraDSO, Engine::HierarchyDSO, Engine::ScriptComponentDSO

#ifdef USER_COMPONENTS_SOURCE
#pragma message("USER_COMPONENTS_SOURCE defined as " USER_COMPONENTS_SOURCE)
#include USER_COMPONENTS_SOURCE
#endif

#ifdef USER_COMPONENTS
#define COMBINED_COMPONENTS ENGINE_COMPONENTS, USER_COMPONENTS
#else
#define COMBINED_COMPONENTS ENGINE_COMPONENTS
#endif

namespace Engine {

struct EntityDSO {
  virtual ~EntityDSO() = default;
};

struct ExplicitEntityDSO : public EntityDSO {
  std::vector<ComponentDSO *> components;
};

// It's a bit ugly to have to have this here, but I'm not sure how else the JSON parsing could be instantiated
// Probably it would be an option to allow specifying a char iterator and use that to explicitly instantiate
// parse_tokenstream for the relevant types, but this will need some rather larger changes in json-parsing I can't
// be bothered to do right now.

struct TransformDSO : public ComponentDSO_T<Graphics::Transform> {
  Maths::Vector3 position;
  Maths::Quaternion rotation;
  Maths::Vector3 scale;

  void FillValues(Graphics::Transform *transform, AssetManager *assetManger) override {
    transform->position = position;
    transform->rotation = rotation;
    transform->scale = scale;
  }
};

struct HierarchyDSO : public ComponentDSO {
  std::vector<EntityDSO *> children;

  void AttachToEntity(Core::Entity entity, AssetManager *assetManger) override {
    throw std::runtime_error("HierarchyComponents are handled explicitly.");
  }
};

struct MeshRendererDSO : public ComponentDSO_T<Graphics::MeshRenderer> {
  std::string meshName;
  std::string materialName;

  void FillValues(Graphics::MeshRenderer *meshRenderer, AssetManager *assetManager) override {
    meshRenderer->mesh = assetManager->LoadAsset<Graphics::AllocatedMesh *>(meshName);
    meshRenderer->material = assetManager->LoadAsset<Graphics::Material *>(materialName);
  }
};

struct CameraDSO : public ComponentDSO_T<Graphics::Camera> {
  float fov;
  float nearClip;
  float farClip;
  float aspectRatio;

  void FillValues(Graphics::Camera *camera, AssetManager *assetManger) override {
    camera->projection = Maths::Transformations::Perspective(nearClip, farClip, fov, aspectRatio);
  }
};

struct PrefabDSO : public EntityDSO {
  std::string prefabName;
  TransformDSO transform;
};

class EntityConverter {
  Core::ECS *ecs;
  AssetManager *assetManager;

public:
  EntityConverter(AssetManager *assetManager, Core::ECS *ecs) : ecs(ecs), assetManager(assetManager) {}
  Core::Entity ConvertDSO(ExplicitEntityDSO const &) const;
};

using EntityLoader = AssetLoaderImpl<Core::Entity, ExplicitEntityDSO, JsonParser<ExplicitEntityDSO>, EntityConverter>;

class EntityDestroyer {
public:
  void DestroyAsset(Core::Entity asset) const { asset.Destroy(); }
};

class EntityManager {
  Core::ECS ecs;
  AssetCacheImpl<Core::Entity, EntityDestroyer> cache;
  EntityLoader loader;
  AssetManager
      *assetManager; // Not needed directly, but for copy constructor (otherwise there's problems with the ecs pointer)

public:
  EntityManager(AssetManager *assetManager) : ecs(), cache(), loader(assetManager, &ecs), assetManager(assetManager) {}
  EntityManager(EntityManager const &other) : EntityManager(other.assetManager) {}
  Core::Entity LoadAsset(char const *assetName);
  inline void Cleanup() { cache.Clear(); }
};

struct SceneDSO {
  std::vector<EntityDSO *> entities;
  int mainCamId;
};

class SceneConverter {
  AssetManager *assetManager;

public:
  SceneConverter(AssetManager *assetManager) : assetManager(assetManager) {}
  Core::Scene *ConvertDSO(SceneDSO const &) const;
};

struct SceneCache {
  AssetCacheImpl<Core::Scene *, OwnedDestroyer<Core::Scene>> baseCache;

  SceneCache() : baseCache() {}
  inline bool HasAsset(char const *assetName) const { return baseCache.HasAsset(assetName); }
  inline void InsertAsset(char const *assetName, Core::Scene *const &asset) { baseCache.InsertAsset(assetName, asset); }
  Core::Scene *LoadAsset(char const *assetName) const;
  void Clear() { baseCache.Clear(); }
};

using SceneLoader = AssetLoaderImpl<Core::Scene *, SceneDSO, JsonParser<SceneDSO>, SceneConverter>;
using SceneManager = TypeManagerImpl<Core::Scene *, SceneLoader, SceneCache>;

#include "ScriptParsing.h"

struct ScriptComponentDSO : public ComponentDSO_T<Core::ScriptComponent> {
  std::vector<ScriptDSO *> scripts;

  void FillValues(Core::ScriptComponent *scriptComponent, AssetManager *assetManager) override {
    for (auto scriptDSO : scripts) {
      scriptDSO->Attach(scriptComponent, assetManager);
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

JSON(Engine::ExplicitEntityDSO, FIELDS(components));
JSON(Engine::PrefabDSO, FIELDS(prefabName, transform));
JSON(Engine::EntityDSO *, SUBTYPES(Engine::ExplicitEntityDSO, Engine::PrefabDSO));

JSON(Engine::SceneDSO, FIELDS(entities, mainCamId));

#ifdef USER_SCRIPTS
JSON(Engine::ScriptDSO *, SUBTYPES(USER_SCRIPTS));
#else
JSON(Engine::ScriptDSO *);
#endif