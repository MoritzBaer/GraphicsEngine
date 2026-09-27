#pragma once

#include "Core/ECS.h"
#include "Maths/Matrix.h"
#include "Maths/Transformations.h"

namespace Engine::Graphics {
struct Camera : public Core::Component {
  Maths::Matrix4 projection;
  float aspectRatio;

  Camera(Core::Entity entity) : Core::Component(entity) {
    projection = Maths::Transformations::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f);
    aspectRatio = 16.0f / 9.0f;
  }

  void CopyFrom(Core::Component const *other) override;

  inline void SetAspectRatio(float newAspectRatio) {
    this->projection[0][0] *= this->aspectRatio / newAspectRatio;
    this->aspectRatio = newAspectRatio;
  }
};


} // namespace Engine::Graphics
