#pragma once

#include "Editor/ImGUIManager.h"
#include "Engine/Core/ECS.h"

namespace Editor {
class EntityDetails : public Engine::Graphics::ImGUIView {

  Engine::Core::Entity const *selectedEntity;
  bool active = true;
  bool oldActive = true;

public:
  EntityDetails(Engine::Core::Entity const *selectedEntity) : ImGUIView(), selectedEntity(selectedEntity) {}
  void Draw();
};
} // namespace Editor
