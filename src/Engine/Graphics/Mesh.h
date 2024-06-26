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
  glm::mat3 TBN;
  glm::vec3 position;
  glm::vec2 uv;
};

/*
struct VertexFormat {
  // Maths::Vector4 TBNP_0;
  Maths::Vector3 position;
  float uv_x;
  Maths::Vector3 normal;
  float uv_y;
  // Maths::Vector4 TBNP_1;
  // Maths::Vector4 TBNP_2;
  Maths::Vector2 uv;

  // VertexFormat() : TBNP_0(), TBNP_1(), TBNP_2(), uv() {}
  // VertexFormat(Vertex const &v)
  //     : TBNP_0(v.TBN[0][X], v.TBN[1][X], v.TBN[2][X], v.position[X]),
  //       TBNP_1(v.TBN[0][Y], v.TBN[1][Y], v.TBN[2][Y], v.position[Y]),
  //       TBNP_2(v.TBN[0][Z], v.TBN[1][Z], v.TBN[2][Z], v.position[Z]), uv(v.uv) {}
  VertexFormat() : position(), uv_x(), normal(), uv_y(), uv() {}
  VertexFormat(Vertex const &v) : position(v.position), uv_x(v.uv[X]), normal(v.TBN[2]), uv_y(v.uv[Y]), uv(uv) {}
};
*/

struct VertexFormat {
  // Maths::Vector4 TBNP_0;
  glm::vec3 position;
  float uv_x;
  glm::vec3 normal;
  float uv_y;
  // Maths::Vector4 TBNP_1;
  // Maths::Vector4 TBNP_2;
  glm::vec3 uv;

  // VertexFormat() : TBNP_0(), TBNP_1(), TBNP_2(), uv() {}
  // VertexFormat(Vertex const &v)
  //     : TBNP_0(v.TBN[0][X], v.TBN[1][X], v.TBN[2][X], v.position[X]),
  //       TBNP_1(v.TBN[0][Y], v.TBN[1][Y], v.TBN[2][Y], v.position[Y]),
  //       TBNP_2(v.TBN[0][Z], v.TBN[1][Z], v.TBN[2][Z], v.position[Z]), uv(v.uv) {}
  VertexFormat() : position(), uv_x(), normal(), uv_y(), uv() {}
  VertexFormat(Vertex const &v) : position(v.position), uv_x(v.uv[X]), normal(v.TBN[2]), uv_y(v.uv[Y]), uv(uv) {}
};

using Mesh = MeshT<Vertex, VertexFormat>;

} // namespace Engine::Graphics
