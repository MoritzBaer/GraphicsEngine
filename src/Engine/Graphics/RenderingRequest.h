#pragma once

#include "Graphics/Camera.h"
#include "Graphics/DrawData.h"
#include "Graphics/MeshRenderer.h"
#include <vector>

namespace Engine::Graphics {
struct RenderingRequest {
  std::vector<MeshRenderer const *> objectsToDraw;
  Camera const *camera;
  SceneData sceneData;
};

} // namespace Engine::Graphics
