#pragma once

#include "Editor/ImGUIManager.h"
#include "Engine/Core/ECS.h"

namespace Editor {
class EntityDetails : public Engine::Graphics::ImGUIView {

  Engine::Core::Entity const *selectedEntity;

public:
  EntityDetails(Engine::Graphics::ImGUIManager &imGuiManager, Engine::Core::Entity const *selectedEntity)
      : ImGUIView(imGuiManager), selectedEntity(selectedEntity) {}
  void Draw() const;
};
} // namespace Editor
