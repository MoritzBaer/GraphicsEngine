#pragma once

#include "Maths/Transformations.h"

#include "glm/gtx/transform.hpp"
#include "glm/matrix.hpp"

#include "Debug/Logging.h"
#include "Util/Macros.h"

#include <cmath>
#include <sstream>

#define EQUALITY_EPS 0.0001

#define TEST_SCOPE(label) TestEnv env = TestEnv(label);

#define TEST_ASSERT(expression, message, ...)                                                                          \
  if (!(expression)) {                                                                                                 \
    env.Fail();                                                                                                        \
    Debug::Logging::PrintError("Test", message, __VA_ARGS__);                                                          \
  }

#define TEST_ASSERT_EQUAL(object1, label1, object2, label2, message)                                                   \
  size_t VAR_WITH_LINE(size) = std::min(sizeof(object1), sizeof(object2)) / sizeof(float);                             \
  if (!test_float_ptr_equal((float *)&object1, (float *)&object2, VAR_WITH_LINE(size))) {                              \
    env.Fail();                                                                                                        \
    Debug::Logging::PrintError("Test", message);                                                                       \
    Debug::Logging::PrintError("Test", "{}", format_single_float_ptr((float *)&object1, label1, VAR_WITH_LINE(size))); \
    Debug::Logging::PrintError("Test", "{}", format_single_float_ptr((float *)&object2, label2, VAR_WITH_LINE(size))); \
  }

using namespace Engine::Maths;

namespace Engine::Test {

class TestEnv {
  const char *label;
  bool passed;

public:
  TestEnv(const char *lbl) : label(lbl), passed(true) {}
  ~TestEnv() {
    if (passed) {
      Debug::Logging::PrintSuccess("Test", "Passed all {} tests!", label);
    } else {
      Debug::Logging::PrintError("Test", "");
      Debug::Logging::PrintError("Test", "Some {} tests failed!", label);
    }
  }

  void Fail() { passed = false; }
};

bool test_float_ptr_equal(float const *p1, float const *p2, uint16_t numberOfFloats) {
  for (int i = 0; i < numberOfFloats; i++) {
    if (abs(p1[i] - p2[i]) > 0.0001f) {
      return false;
    }
  }
  return true;
}

std::string format_single_float_ptr(float const *ptr, const char *lbl, uint16_t numberOfFloats) {
  std::stringstream builder;

  builder << std::fixed;
  builder.precision(6);

  for (int i = 0; i < numberOfFloats; i++) {
    float val = ptr[i];
    if (!std::signbit(val)) {
      builder << " ";
    }
    builder << val << " ";
  }
  builder << " (" << lbl << ")";

  return builder.str();
}

void VectorTests() {
  TEST_SCOPE("vector")

  Vector<10> v1{20, 34, 95, 35, 98, 84, 20, 85, 18, 70};
  Vector4 v2 = v1.xyzw();
  TEST_ASSERT_EQUAL(v1, "vector with ten entries", v2, "first four entries",
                    "Accessor for first four entries did not give the correct values!")

  glm::vec4 glmv2(v2[X], v2[Y], v2[Z], v2[W]);
  TEST_ASSERT_EQUAL(v2, "own", glmv2, "glm", "GLM representation doesn't match internal representation!")
  float v2l = v2.Length();
  float glmv2l = glm::length(glmv2);
  TEST_ASSERT_EQUAL(v2l, "own", glmv2l, "glm", "Length calculation gives wrong result!")

  v2.Normalize();
  TEST_ASSERT(abs(v2.Length() - 1) < EQUALITY_EPS, "Normalization results in incorrect length! ({})", v2.Length())
  glmv2 = glm::normalize(glmv2);
  TEST_ASSERT_EQUAL(v2, "own", glmv2, "glm", "Normalization gives incorrect result!")

  v1.xyzw() = v2;
  TEST_ASSERT_EQUAL(v1, "Vector10", v2, "Vector4", "Setting entries does not give correct result!")

  Vector3 a{0.36309, 2.67769, 1.92708};
  Vector3 b{2.23122, 1.42397, 1.84785};
  glm::vec3 glma = glm::vec3(0.36309, 2.67769, 1.92708);
  glm::vec3 glmb = glm::vec3(2.23122, 1.42397, 1.84785);
  Vector3 axb = a.Cross(b);
  glm::vec3 glmaxb = glm::cross(glma, glmb);
  TEST_ASSERT_EQUAL(axb, "own", glmaxb, "glm", "Cross product gives incorrect result!")
}

void MatrixTests() {
  TEST_SCOPE("matrix")
  Matrix4 m1{8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015,
             1.89949, 7.79529, 0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304};
  glm::mat4 glm1 = glm::mat4(8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015, 1.89949, 7.79529,
                             0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304);
  TEST_ASSERT_EQUAL(m1, "own", glm1, "glm", "Matrix representation does not match!")

  Matrix4 m2{8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015,
             1.89949, 7.79529, 0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304};
  glm::mat4 glm2 = glm::mat4(8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015, 1.89949, 7.79529,
                             0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304);

  Matrix4 m3 = m1 * m2;
  glm::mat4 glm3 = glm1 * glm2;
  TEST_ASSERT_EQUAL(m3, "own", glm3, "glm", "Matrix multiplication does not give the correct result!")
}

void TransformTests() {
  TEST_SCOPE("transformation")

  auto r1 = Transformations::Rotate({0.92092, 1.58194, 0.77804}, 2.62435f);
  auto glmr1 = glm::rotate(2.62435f, glm::vec3(0.92092, 1.58194, 0.77804));
  TEST_ASSERT_EQUAL(r1, "own", glmr1, "glm", "Rotation matrix is incorrect!")

  auto l = Transformations::LookAt(Vector3{2.65826, 2.44898, 0.85060}, {7.51053, 15.33626, 10.05824}, {0, 0, 1});
  auto glm =
      glm::lookAt(glm::vec3(2.65826, 2.44898, 0.85060), glm::vec3(7.51053, 15.33626, 10.05824), glm::vec3(0, 0, 1));
  TEST_ASSERT_EQUAL(l, "own", glm, "glm", "LookAt matrix is incorrect!")

  auto p = Transformations::Perspective(0.01, 1000.0, 45.0, 16.0 / 9.0);
  auto glmp = glm::perspective(glm::radians(45.0), 16.0 / 9.0, 0.01, 1000.0);
  glmp[1] *= -1; // OpenGL and Vulkan are of two minds regarding the y axis orientation
  TEST_ASSERT_EQUAL(p, "own", glmp, "glm", "Perspective matrix is incorrect!")
}

void QuaternionTests() {
  TEST_SCOPE("quaternion")

  Vector3 axis1{0.73059, 1.19045, 0.55717};
  Vector3 axis2{0.73404, 0.95724, 0.69343};
  Vector3 point{3.71460, 12.14657, 3.76972};
  float angle = 13.0125f;
  Quaternion q = Transformations::RotateAroundAxis(axis1, angle);
  auto cq = q.RotationMatrix();
  auto mq = Transformations::RodriguesRotation(axis1, angle);
  TEST_ASSERT_EQUAL(cq, "quaternion", mq, "rodriguez",
                    "Conversion from quaternion to rotation matrix gives incorrect result!")

  Quaternion r = Transformations::RotateAroundAxis(axis2, 0.32f);
  auto mr = Transformations::RodriguesRotation(axis2, 0.32f);
  auto cr = r.RotationMatrix();
  TEST_ASSERT_EQUAL(cr, "quaternion", mr, "rodriguez",
                    "Conversion from quaternion to rotation matrix gives incorrect result!")
  Quaternion p = q * r; // Quaternion multiplication and matrix multiplication have inverse orders
  auto mp = mr * mq;
  auto cp = p.RotationMatrix();
  TEST_ASSERT_EQUAL(cp, "quaternion", mp, "rodriguez", "Quaternion multiplication gives incorrect result!")

  auto pp = Transformations::RotateByQuaternion(point, p);
  auto pm = mp * point;
  TEST_ASSERT_EQUAL(pp, "quaternion", pm, "matrix",
                    "Matrix-point rotation and quaternion-point-rotation not equivalent!")

  auto pq = Transformations::RotateByQuaternion(point, q);
  auto pqr = Transformations::RotateByQuaternion(pq, r);
  TEST_ASSERT_EQUAL(pp, "direct", pqr, "indirect",
                    "Quaternion rotation separation is not equivalent to in-one-go-rotation!")
}

void RunMathsTests() {
  VectorTests();
  MatrixTests();
  TransformTests();
  QuaternionTests();
}
} // namespace Engine::Test
