#pragma once

#include "ECS.h"

#include "Util/Macros.h"

namespace Engine::Core {
class SceneHierarchy {
  ECS *ecs;

  struct TreeNode;
  TreeNode BuildNode(Entity const &e);

public:
  SceneHierarchy(ECS *ecs) : ecs(ecs) {}
  void Rebuild();

  struct TreeNode {
  private:
    std::vector<TreeNode> children;

  public:
    Entity entity;
    inline bool HasChildren() const { return !children.empty(); }
    inline std::vector<TreeNode>::const_iterator begin() const { return children.begin(); }
    inline std::vector<TreeNode>::const_iterator end() const { return children.end(); }

    friend TreeNode SceneHierarchy::BuildNode(Entity const &e);
  };

  std::vector<TreeNode> roots;

  inline std::vector<TreeNode>::const_iterator begin() const { return roots.begin(); }
  inline std::vector<TreeNode>::const_iterator end() const { return roots.end(); }
}; // class SceneHierarchy

} // namespace Engine::Core
