#pragma once

#include "ECS.h"

#include "Util/Macros.h"

namespace Engine::Core {
namespace SceneHierarchy {
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

inline class RootEntityContainer {
  std::vector<TreeNode> roots;

public:
  friend void SceneHierarchy::BuildHierarchy();
  inline std::vector<TreeNode>::const_iterator begin() { return roots.begin(); }
  inline std::vector<TreeNode>::const_iterator end() { return roots.end(); }
} RootEntities;
} // namespace SceneHierarchy

} // namespace Engine::Core
