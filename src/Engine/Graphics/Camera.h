#include "Core/ECS.h"
#include "Maths/Transformations.h"

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Camera) {
  Maths::Matrix4 projection;
  ENGINE_COMPONENT_CONSTRUCTOR(Camera) {
    projection = Maths::Transformations::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f);
  }
};

} // namespace Engine::Graphics
