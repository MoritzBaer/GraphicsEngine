#pragma once

#include "Buffer.h"
#include "CommandQueue.h"
#include "Maths/Matrix.h"
#include <algorithm>

namespace Engine::Graphics {
// T_GPU must have a constructor taking a T_CPU const &
template <typename T_CPU, typename T_GPU> class MeshT {

public:
  std::vector<T_CPU> vertices;
  std::vector<uint32_t> indices;

  inline std::vector<T_GPU> ReformattedVertices() {
    std::vector<T_GPU> reformattedVerts(vertices.size());
    std::transform(vertices.begin(), vertices.end(), reformattedVerts.begin(), [](T_CPU const &v) { return T_GPU(v); });
    return reformattedVerts;
  }
};

struct Vertex {
  Maths::Vector3 position;
  Maths::Vector2 uv;
  Maths::Vector3 normal;
  Maths::Vector4 colour;
};

struct VertexFormat {
  Maths::Vector3 position;
  float uv_x;
  Maths::Vector3 normal;
  float uv_y;
  Maths::Vector4 colour;

  VertexFormat() : position(), uv_x(), normal(), uv_y(), colour() {}
  VertexFormat(Vertex const &v)
      : position(v.position), uv_x(v.uv[X]), normal(v.normal), uv_y(v.uv[Y]), colour(v.colour) {}
};

using Mesh = MeshT<Vertex, VertexFormat>;

} // namespace Engine::Graphics
