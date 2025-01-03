#pragma once

#include "ECS.h"

#include "Util/Macros.h"

namespace Engine::Core {
class SceneHierarchy {
  ECS *ecs;

public:
  SceneHierarchy(ECS *ecs) : ecs(ecs) {}
  struct TreeNode;

  void BuildHierarchy();
  TreeNode BuildNode(Entity const &e);

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
