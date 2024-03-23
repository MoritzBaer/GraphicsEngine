#pragma once

#include "Buffer.h"
#include "CommandQueue.h"
#include "Maths/Matrix.h"

namespace Engine::Graphics {
class Mesh {

public:
  struct Vertex {
    Maths::Vector3 position;
    Maths::Vector2 uv;
    Maths::Vector3 normal;
    Maths::Vector4 colour;
  };

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

} // namespace Engine::Graphics
