#pragma once

#include "Graphics/ImGUIManager.h"

namespace Engine::Editor {
class EntityDetails : public Graphics::ImGUIView {

public:
  EntityDetails() : ImGUIView() {}
  void Draw() const;
};
} // namespace Engine::Editor
