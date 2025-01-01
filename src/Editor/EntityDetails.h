#pragma once

#include "Editor/ImGUIManager.h"

namespace Editor {
class EntityDetails : public Engine::Graphics::ImGUIView {

public:
  EntityDetails(Engine::Graphics::ImGUIManager &imGuiManager) : ImGUIView(imGuiManager) {}
  void Draw() const;
};
} // namespace Editor
