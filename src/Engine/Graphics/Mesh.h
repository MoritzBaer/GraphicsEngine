#pragma once

#include "Buffer.h"
#include "CommandQueue.h"
#include "Maths/Matrix.h"
#include "glm/matrix.hpp"
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
  Maths::Matrix3 TBN;
  Maths::Vector3 position;
  Maths::Vector2 uv;
};

struct VertexFormat {
  alignas(16) Maths::Vector4 TBNP_0;
  alignas(16) Maths::Vector4 TBNP_1;
  alignas(16) Maths::Vector4 TBNP_2;
  alignas(8) Maths::Vector2 uv;

  VertexFormat() : TBNP_0(), TBNP_1(), TBNP_2(), uv() {}
  VertexFormat(Vertex const &v)
      : TBNP_0(v.TBN[0][X], v.TBN[1][X], v.TBN[2][X], v.position[X]),
        TBNP_1(v.TBN[0][Y], v.TBN[1][Y], v.TBN[2][Y], v.position[Y]),
        TBNP_2(v.TBN[0][Z], v.TBN[1][Z], v.TBN[2][Z], v.position[Z]), uv(v.uv) {}
};

using Mesh = MeshT<Vertex, VertexFormat>;

} // namespace Engine::Graphics
