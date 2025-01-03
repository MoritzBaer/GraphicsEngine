#pragma once

#include "Core/SceneHierarchy.h"
#include "Editor/ImGUIManager.h"

namespace Editor {
class EntityDetails;
class SceneView : public Engine::Graphics::ImGUIView {
  Engine::Core::SceneHierarchy const *sceneHierarchy;
  Engine::Core::Entity *selectedEntity;

  void DrawNode(Engine::Core::SceneHierarchy::TreeNode const &node, ImGuiTreeNodeFlags flags = 0) const;

public:
  SceneView(Engine::Graphics::ImGUIManager &imGuiManager, Engine::Core::SceneHierarchy *sceneHierarchy,
            Engine::Core::Entity *selectedEntity)
      : ImGUIView(imGuiManager), sceneHierarchy(sceneHierarchy), selectedEntity(selectedEntity) {}
  void Draw() const;
};

} // namespace Editor
