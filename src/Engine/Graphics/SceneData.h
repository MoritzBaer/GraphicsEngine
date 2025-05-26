#pragma once

#include "Maths/Matrix.h"

namespace Engine::Graphics {

struct SceneData {
  alignas(16) Maths::Vector3 cameraPosition;
  alignas(16) Maths::Vector3 lightDirection;
  alignas(16) Maths::Vector3 lightColour;
};

} // namespace Engine::Graphics
