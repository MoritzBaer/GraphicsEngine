#include "SceneHierarchy.h"

#include "Graphics/Transform.h"

Engine::Core::SceneHierarchy::TreeNode Engine::Core::SceneHierarchy::BuildNode(Entity const &e) {
  TreeNode node;
  node.entity = e;

  if (e.HasComponent<Graphics::Transform>()) {
    for (auto c : e.GetComponent<Graphics::Transform>()->children) {
      node.children.push_back(BuildNode(c->entity));
    }
  }
  return node;
}

void Engine::Core::SceneHierarchy::BuildHierarchy() {
  roots = std::vector<TreeNode>();
  for (auto e : *ecs) {
    if (!e.HasComponent<Graphics::Transform>() || !e.GetComponent<Graphics::Transform>()->parent) {
      roots.push_back(BuildNode(e));
    }
  }
}
