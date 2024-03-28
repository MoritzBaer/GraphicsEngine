#pragma once

#include "Core/ECS.h"
#include "SceneView.h"

namespace Engine::Editor {
class EntityDetails : public Graphics::ImGUIView {
  Core::Entity entity;

public:
  EntityDetails(SceneView &parent) : ImGUIView() { parent.RegisterEntityDetails(this); }
  inline void SetEntity(Core::Entity const &newEntity) { entity = newEntity; }
  void Draw() const;
};
} // namespace Engine::Editor
