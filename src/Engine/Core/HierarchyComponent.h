#pragma once

#include "Core/ECS.h"
#include "Debug/Logging.h"

class SceneCache;

namespace Engine::Core {

class HierarchyComponent;

class HierarchicalComponent : public Component {
protected:
  HierarchyComponent *hierarchy;
  friend class SceneCache;

public:
  HierarchicalComponent(Entity entity);
  virtual void OnHierarchyChange() = 0;
};

class HierarchyComponent : public Component {
  std::vector<HierarchicalComponent *> hierarchyChangeListeners;

public:
  HierarchyComponent *parent;
  std::vector<HierarchyComponent *> children;

  HierarchyComponent(Entity entity) : Component(entity), parent(nullptr), children() {}
  inline void SetParent(HierarchyComponent *newParent);
  inline void RegisterListener(HierarchicalComponent *listener) { hierarchyChangeListeners.push_back(listener); }
  inline void CopyFrom(Component const *other) override;
};

inline HierarchicalComponent::HierarchicalComponent(Entity entity) : Component(entity) {
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

inline void HierarchyComponent::CopyFrom(Component const *other) {
  if (auto otherHierarchy = dynamic_cast<HierarchyComponent const *>(other)) {
    for (auto child : otherHierarchy->children) {
      auto newChild = child->entity.CopyToOtherECS(entity.parentECS).GetComponent<HierarchyComponent>();
      newChild->parent = this;
      children.push_back(newChild);
    }
  } else {
    ENGINE_ERROR("Tried to copy HierarchyComponent from different type!");
  }
}
} // namespace Engine::Core
