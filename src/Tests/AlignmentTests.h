#pragma once

#include "Maths/Matrix.h"
#include "Test.h"

using namespace Engine::Maths;
using namespace Engine;

TEST_CASE("Maths types are aligned correctly") {

  struct Vec3Alignment {
    Vector3 u;
    Vector3 v;
    Vector3 w;
  };

  struct Vec3AlignmentExplicit {
    alignas(16) float u[3];
    alignas(16) float v[3];
    alignas(16) float w[3];
  };

  VERIFY(sizeof(Vec3Alignment) == sizeof(Vec3AlignmentExplicit))

  struct ExplicitVertex {
    alignas(16) float TBN[9];
    alignas(16) float position[3];
    alignas(8) float uv[2];
  };

  struct Vertex {
    Matrix3 TBN;
    Vector3 position;
    Vector2 uv;
  };

  VERIFY(sizeof(Vertex) == sizeof(ExplicitVertex))

  Vertex v{Matrix3{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f}, Vector3{10.0f, 11.0f, 12.0f},
           Vector2{13.0f, 14.0f}};
  ExplicitVertex ev{{1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f}, {10.0f, 11.0f, 12.0f}, {13.0f, 14.0f}};

  // TODO: Do that then!
  // Has to be done with more nuance apparently
  // VERIFY_MEM_EQUAL(v, ev, float)
}