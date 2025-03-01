#pragma once

#include "Core/ECS.h"
#include "Debug/Logging.h"

class SceneCache;

namespace Engine::Core {

class HierarchyComponent;

class HierarchyListener {
protected:
  HierarchyComponent *hierarchy;
  friend class SceneCache;

public:
  HierarchyListener(HierarchyComponent *hierarchy) : hierarchy(hierarchy) {};
  virtual void OnHierarchyChange() = 0;
};

template <typename T> class HierarchicalComponent : public ComponentT<T>, public HierarchyListener {
public:
  HierarchicalComponent(Entity entity);
  virtual void OnHierarchyChange() override = 0;
};

class HierarchyComponent : public ComponentT<HierarchyComponent> {
  std::vector<HierarchyListener *> hierarchyChangeListeners;

public:
  HierarchyComponent *parent;
  std::vector<HierarchyComponent *> children;

  HierarchyComponent(Entity entity) : ComponentT<HierarchyComponent>(entity), parent(nullptr), children() {}
  inline void SetParent(HierarchyComponent *newParent);
  inline void RegisterListener(HierarchyListener *listener) { hierarchyChangeListeners.push_back(listener); }
  inline void CopyFrom(HierarchyComponent const &other) override;
};

template <typename T>
inline HierarchicalComponent<T>::HierarchicalComponent(Entity entity)
    : ComponentT<T>(entity),
      HierarchyListener(entity.HasComponent<HierarchyComponent>() ? entity.GetComponent<HierarchyComponent>()
                                                                  : entity.AddComponent<HierarchyComponent>()) {
  if (!entity.HasComponent<HierarchyComponent>()) {
    hierarchy = entity.AddComponent<HierarchyComponent>();
  } else {
    hierarchy = entity.GetComponent<HierarchyComponent>();
  }
  hierarchy->RegisterListener(this);
}

void HierarchyComponent::SetParent(HierarchyComponent *newParent) {
  if (parent) {
    parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
  }

  parent = newParent;
  newParent->children.push_back(this);

  for (auto listener : hierarchyChangeListeners) {
    listener->OnHierarchyChange();
  }
}

void HierarchyComponent::CopyFrom(HierarchyComponent const &other) {
  for (auto child : other.children) {
    auto newChild = child->entity.CopyToOtherECS(entity.parentECS).GetComponent<HierarchyComponent>();
    newChild->parent = this;
    children.push_back(newChild);
  }
}

} // namespace Engine::Core
