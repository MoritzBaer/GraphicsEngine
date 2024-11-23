#pragma once

#include "Graphics/ImGUIManager.h"

namespace Engine::Editor {
class EntityDetails : public Graphics::ImGUIView {

public:
  EntityDetails(Graphics::ImGUIManager &imGuiManager) : ImGUIView(imGuiManager) {}
  void Draw() const;
};
} // namespace Engine::Editor
