#pragma once

#include "Maths/Matrix.h"
#include "Test.h"
#include "glm/matrix.hpp"

using namespace Engine::Maths;

namespace Engine::Test {

BEGIN_TEST_CASE(alignment)

struct Vec3Alignment {
  Vector3 u;
  Vector3 v;
  Vector3 w;
};

struct Vec3AlignmentGLM {
  glm::vec3 u;
  glm::vec3 v;
  glm::vec3 w;
};

TEST_ASSERT(sizeof(Vec3Alignment) == sizeof(Vec3AlignmentGLM), "Vector3 not aligned correctly!")

struct glmVertex {
  glm::mat3 TBN;
  glm::vec3 position;
  glm::vec2 uv;
};

struct Vertex {
  Matrix3 TBN;
  Vector3 position;
  Vector2 uv;
};

TEST_ASSERT(sizeof(glmVertex) == sizeof(Vertex), "Vertex not aligned correctly!")

Vertex v{Matrix3{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f}, Vector3{10.0f, 11.0f, 12.0f},
         Vector2{13.0f, 14.0f}};
glmVertex gv{glm::mat3(1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f), glm::vec3(10.0f, 11.0f, 12.0f),
             glm::vec2(13.0f, 14.0f)};
TEST_ASSERT_EQUAL(float, v, "own", gv, "glm", "Vertex not aligned correctly!")

END_TEST_CASE() // alignment

} // namespace Engine::Test