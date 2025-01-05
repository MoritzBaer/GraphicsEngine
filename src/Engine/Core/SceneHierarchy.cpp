#include "SceneHierarchy.h"

#include "Core/HierarchyComponent.h"

Engine::Core::SceneHierarchy::TreeNode Engine::Core::SceneHierarchy::BuildNode(Entity const &e) {
  TreeNode node;
  node.entity = e;

  if (e.HasComponent<Core::HierarchyComponent>()) {
    for (auto c : e.GetComponent<Core::HierarchyComponent>()->children) {
      node.children.push_back(BuildNode(c->entity));
    }
  }
  return node;
}

void Engine::Core::SceneHierarchy::Rebuild() {
  roots = std::vector<TreeNode>();
  for (auto e : *ecs) {
    if (!e.HasComponent<Core::HierarchyComponent>() || !e.GetComponent<Core::HierarchyComponent>()->parent) {
      roots.push_back(BuildNode(e));
    }
  }
}
