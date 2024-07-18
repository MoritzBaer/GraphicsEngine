#pragma once

#include "Graphics/ImGUIManager.h"

namespace Engine::Editor {
class EntityDetails;
class SceneView : public Graphics::ImGUIView {

public:
  SceneView() : ImGUIView() {}
  void Draw() const;
};

} // namespace Engine::Editor
