#pragma once

#include "Core/ECS.h"
#include "Maths/Transformations.h"

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Camera) {
  Maths::Matrix4 projection;
  ENGINE_COMPONENT_CONSTRUCTOR(Camera) {
    projection = Maths::Transformations::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f);
  }

  inline void CopyFrom(Core::Component const *other) override {
    if (auto otherCamera = dynamic_cast<Camera const *>(other)) {
      projection = otherCamera->projection;
    } else {
      ENGINE_ERROR("Tried to copy Camera from different type!");
    }
  }
};

} // namespace Engine::Graphics
