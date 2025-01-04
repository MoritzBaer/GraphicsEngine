#pragma once

#include "Core/ECS.h"
#include "Maths/Transformations.h"

namespace Engine::Graphics {
struct Camera : public Core::Component {
  Maths::Matrix4 projection;
  Camera(Core::Entity entity) : Core::Component(entity) {
    projection = Maths::Transformations::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f);
  }

  void CopyFrom(Core::Component const *other) override;
};

} // namespace Engine::Graphics
