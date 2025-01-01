#pragma once

#include "Core/SceneHierarchy.h"
#include "Editor/ImGUIManager.h"

namespace Editor {
class EntityDetails;
class SceneView : public Engine::Graphics::ImGUIView {
  Engine::Core::SceneHierarchy &sceneHierarchy;

public:
  SceneView(Engine::Graphics::ImGUIManager &imGuiManager, Engine::Core::SceneHierarchy &sceneHierarchy)
      : ImGUIView(imGuiManager), sceneHierarchy(sceneHierarchy) {}
  void Draw() const;
};

} // namespace Editor
