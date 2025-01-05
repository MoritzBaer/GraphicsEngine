#pragma once

#include "Core/ECS.h"
#include "Core/SceneHierarchy.h"

namespace Engine::Core {

struct Scene {
  ECS ecs;
  SceneHierarchy sceneHierarchy;
  Entity mainCamera;

  Scene() : ecs(), sceneHierarchy(&ecs), mainCamera() {};
  inline Entity InstantiateEntity(Entity const &entity) {
    auto instance = entity.CopyToOtherECS(&ecs);
    sceneHierarchy.Rebuild();
    return instance;
  }
};

} // namespace Engine::Core
