#include "Core/ECS.h"
#include "Maths/Transformations.h"

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Camera) {
  Maths::Matrix4 viewProjection;
  ENGINE_COMPONENT_CONSTRUCTOR(Camera) {
    viewProjection = Maths::Transformations::Perspective(0.01f, 100.0f, 45.0f, 16.0f / 9.0f) *
                     Maths::Transformations::LookAt(Maths::Vector3{0, 0, 5}, {0, 0, 0},
                                                    {0, 1, 0}); // I have absolutely no idea why the first initializer
                                                                // list needs to have a type and the others don't...
  }
};

} // namespace Engine::Graphics
