#pragma once

#include "Maths/Matrix.h"

namespace Engine::Graphics {

struct SceneData {
  Maths::Vector3 lightDirection;
  Maths::Vector3 lightColour;
};

struct DrawData {
  Maths::Matrix4 view;
  Maths::Matrix4 projection;
  Maths::Matrix4 viewProjection;
  SceneData sceneData;
};

} // namespace Engine::Graphics
