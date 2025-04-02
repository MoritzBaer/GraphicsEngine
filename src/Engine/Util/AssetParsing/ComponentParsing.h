#pragma once

#include "Core/ECS.h"

namespace Engine {
struct ComponentDSO {
  virtual void AttachToEntity(Core::Entity entity, AssetManager *assetManger) = 0;
};

template <typename T> struct ComponentDSO_T : public ComponentDSO {
  void AttachToEntity(Core::Entity entity, AssetManager *assetManger) override {
    FillValues(entity.AddComponent<T>(), assetManger);
  }
  virtual void FillValues(T *component, AssetManager *assetManger) = 0;
};

} // namespace Engine