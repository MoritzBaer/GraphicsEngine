#pragma once

#include "Core/SceneHierarchy.h"
#include "Graphics/ImGUIManager.h"

namespace Engine::Editor {
class EntityDetails;
class SceneView : public Graphics::ImGUIView {
  Core::SceneHierarchy &sceneHierarchy;

public:
  SceneView(Graphics::ImGUIManager &imGuiManager, Core::SceneHierarchy &sceneHierarchy)
      : ImGUIView(imGuiManager), sceneHierarchy(sceneHierarchy) {}
  void Draw() const;
};

} // namespace Engine::Editor
