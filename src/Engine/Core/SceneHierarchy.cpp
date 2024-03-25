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
  RootEntities.roots = std::vector<TreeNode>();
  for (auto e : ECS::Entities) {
    if (!e.HasComponent<Graphics::Transform>() || !e.GetComponent<Graphics::Transform>()->parent) {
      RootEntities.roots.push_back(BuildNode(e));
    }
  }
}
