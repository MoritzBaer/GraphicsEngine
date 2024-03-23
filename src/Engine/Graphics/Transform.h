#pragma once

#include "Core/ECS.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Transform) {
  Maths::Matrix4 modelMatrix;
  Transform *parent;
  std::vector<Transform *> children;

  ENGINE_COMPONENT_CONSTRUCTOR(Transform), modelMatrix(Maths::Matrix4::Identity()), parent(nullptr), children() {}

  inline void SetParent(Transform * newParent);
};

inline void Transform::SetParent(Transform *newParent) {}

} // namespace Engine::Graphics
