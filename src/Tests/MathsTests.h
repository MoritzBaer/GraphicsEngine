#pragma once

#include "Test.h"

#include "Maths/Transformations.h"

#include "Debug/Logging.h"

using namespace Engine::Maths;
using namespace Engine;

namespace Test {

BEGIN_TEST_CASE(vector)

Vector<10> v1{20, 34, 95, 35, 98, 84, 20, 85, 18, 70};
Vector4 v2 = v1.xyzw();
TEST_ASSERT_EQUAL(float, v1, "vector with ten entries", v2, "first four entries",
                  "Accessor for first four entries did not give the correct values!")

float explicit_v2[] = {20, 34, 95, 35};
TEST_ASSERT_EQUAL(float, v2, "actual", explicit_v2, "wanted", "Internal representation is not as desired!")
float v2l = v2.Length();
float expected_length = 108.655418641f;
TEST_ASSERT_EQUAL(float, v2l, "actual", expected_length, "wanted", "Length calculation gives wrong result!")

v2.Normalize();
TEST_ASSERT(abs(v2.Length() - 1) < EQUALITY_EPS, "Normalization results in incorrect length of {}!", v2.Length())
float correct_v2[] = {explicit_v2[0] / v2l, explicit_v2[1] / v2l, explicit_v2[2] / v2l, explicit_v2[3] / v2l};
TEST_ASSERT_EQUAL(float, v2, "actual", correct_v2, "correct", "Normalization gives incorrect result!")

v1.xyzw() = v2;
TEST_ASSERT_EQUAL(float, v1, "Vector10", v2, "Vector4", "Setting entries does not give correct result!")

Vector3 a{1, 2, 3};
Vector3 b{3, 4, 5};
Vector3 axb = a.Cross(b);
Vector3 correct_axb = {-2, 4, -2};
TEST_ASSERT_EQUAL(float, axb, "actual", correct_axb, "correct", "Cross product gives incorrect result!")

END_TEST_CASE() // vector

BEGIN_TEST_CASE(matrix)

MatrixNM<2, 3> A = MatrixNM<2, 3>(1, 2, 3, 3, 2, 1);
Matrix3 B{4, 1, 0, 0, 3, 0, 2, 1, 2};

MatrixNM<2, 3> C{10, 10, 6, 14, 10, 2};
MatrixNM<2, 3> D = A * B;
TEST_ASSERT_EQUAL(float, D, "actual", C, "correct", "Matrix multiplication does not give the correct result!")

Matrix4 M{1, 3, 2, 4, 0, 1, 1, 3, 2, 0, 0, 2, 1, 2, 4, 1};
Vector4 x{4, 2, 3, 0};
Vector4 y = (M * 2.5f) * (x * 2.5f);
Vector4 y_ = Vector4{16, 5, 8, 20} * (2.5f * 2.5f);
TEST_ASSERT_EQUAL(float, y, "actual", y_, "correct", "Matrix-vector multiplication does not give the correct result!")

{
  Matrix2 M = Matrix2(1, 1, 2, 3);
  Matrix2 M_Inv = M.Inverse();
  Matrix2 M_Exp = Matrix2(3, -1, -2, 1);
  TEST_ASSERT_EQUAL(float, M_Inv, "actual", M_Exp, "correct", "Matrix inversion does not give the correct result!")
}
{
  Matrix3 M = Matrix3(1, 2, 3, 4, 5, 6, 7, 8, 9);
  float explicit_M[] = {1, 4, 7, 2, 5, 8, 3, 6, 9};
  TEST_ASSERT_EQUAL(float, M, "actual", explicit_M, "wanted", "Matrix representation does not match for 3x3 matrices!")

  Matrix3 const &m = M;
  float m_12 = m[1][2];
}

// The GPU expects matrices to be in column-major order, but for easier notation, constructors expect row-major
Matrix4 m1 = Matrix4(8.96836, 2.10230, 1.89949, 4.60040, 8.39039, 5.83538, 7.79529, 6.32591, 4.75992, 5.49344, 0.39212,
                     0.78537, 6.66451, 8.18015, 3.31983, 4.87304);
float explicit_m1[] = {8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015,
                       1.89949, 7.79529, 0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304};
TEST_ASSERT_EQUAL(float, m1, "actual", explicit_m1, "explicit", "Matrix representation does not match!")

Matrix4 m2{5.03589, 6.78238, 7.17521, 3.66256, 8.01326, 4.52237, 6.94845, 0.59203,
           5.60207, 2.59969, 7.80068, 7.32192, 0.61612, 5.25223, 6.61894, 3.31925};

Maths::Vector4 v1{9.85868, 1.70382, 6.27017, 6.29855};
Maths::Vector4 v2 = m1 * v1;
Maths::Vector4 correct_v2 = {132.8841067841, 181.3824611666, 63.6917123203, 131.1497591829};
TEST_ASSERT_EQUAL(float, v2, "actual", correct_v2, "correct",
                  "Matrix-vector multiplication does not give the correct result!")

Matrix4 m3 = m1 * m2;
Matrix4 correct_m3 = {75.4854253307,  99.4346479979,  124.2246780198, 63.2695727914, 136.5807781554, 136.7870325182,
                      203.4290379255, 112.2587933441, 70.671362396,   62.2712887803, 80.5814383006,  26.1636845213,
                      120.7117055058, 116.4197600212, 162.8099129266, 69.7344296437};
TEST_ASSERT_EQUAL(float, m3, "actual", correct_m3, "correct", "Matrix multiplication does not give the correct result!")

Matrix4 m4 = Matrix4(6.01589, 0.79945, 3.03696, 7.89833, 2.89674, 6.73960, 5.22906, 1.90390, 4.23525, 3.65487, 7.92008,
                     6.07612, 0.21322, 7.01907, 0.17421, 7.19930);
Matrix4 m_Inverse = m4.Inverse();
Matrix4 m_correct =
    Matrix4(0.22590520535147221064, 0.21439106151673295909, -0.22566118257648591511, -0.11408172710246326558,
            -0.0048146176943861615186, 0.15176869603639391364, -0.099435098775642683715, 0.0490679109584360566,
            -0.11926301806954870517, -0.067545549318803498536, 0.21735238026807963992, -0.034736779955989534575,
            0.00088945329587463363156, -0.15268435171168592384, 0.098369624539275549675, 0.095282166115857150355);

TEST_ASSERT_EQUAL(float, m_Inverse, "actual", m_correct, "correct",
                  "Matrix inversion does not give the correct result!");

END_TEST_CASE() // Matrix

BEGIN_TEST_CASE(perspective)

auto P = Transformations::Perspective(0.01f, 100.0f, 60.0f, 16.0f / 9.0f);

Vector4 v1 = {1, 10, 0, 1};
Vector4 v1p = P * v1;
Vector3 v1h = v1p.xyz();
v1h /= v1p[W];

TEST_ASSERT(v1h[X] > 0 && v1h[X] < 1 && v1h[Y] == 0 && v1h[Z] > 0 && v1h[Z] < 1,
            "Perspective matrix does not transform point in front and right of camera correctly! (homogenized x: {})",
            v1h[X])

Vector4 v2 = {0, 11, 0, 1};
Vector4 v2p = P * v2;
Vector3 v2h = v2p.xyz();
v2h /= v2p[W];

TEST_ASSERT(v2h[X] == 0 && v2h[Y] == 0 && v2h[Z] > 0 && v2h[Z] < 1,
            "Perspective matrix does not transform point in front of camera correctly! (homogenized z: {})", v2h[Z])

Vector4 v3 = {0, 10, 1, 1};
Vector4 v3p = P * v3;
Vector3 v3h = v3p.xyz();
v3h /= v3p[W];

TEST_ASSERT(v3h[X] == 0 && v3h[Y] < 0 && v3h[Y] > -1 && v3h[Z] > 0 && v3h[Z] < 1,
            "Perspective matrix does not transform point in front and above of camera correctly! (homogenized y: {})",
            v3h[Y])

Vector4 v4 = {0.01, 1.99, 0.01, 1};
Vector4 v4p = P * v4;
Vector3 v4h = v4p.xyz();
v4h /= v4p[W];

TEST_ASSERT(
    v4h[X] > 0 && v4h[X] < 1 && v4h[Y] < 0 && v4h[Y] > -1 && v4h[Z] > 0 && v4h[Z] < 1,
    "Perspective matrix does not transform point in front and right of camera correctly! (homogenized coordinates: {})",
    v4h)

END_TEST_CASE()

BEGIN_TEST_CASE(quaternion)

Vector3 axis1{0.73059, 1.19045, 0.55717};
Vector3 axis2{0.73404, 0.95724, 0.69343};
Vector3 point{3.71460, 12.14657, 3.76972};
float angle = 13.0125f;
Quaternion q = Transformations::RotateAroundAxis(axis1, angle);
auto cq = q.RotationMatrix();
auto mq = Transformations::RodriguesRotation(axis1, angle);
TEST_ASSERT_EQUAL(float, cq, "quaternion", mq, "rodriguez",
                  "Conversion from quaternion to rotation matrix gives incorrect result!")

Quaternion r = Transformations::RotateAroundAxis(axis2, 0.32f);
auto mr = Transformations::RodriguesRotation(axis2, 0.32f);
auto cr = r.RotationMatrix();
TEST_ASSERT_EQUAL(float, cr, "quaternion", mr, "rodriguez",
                  "Conversion from quaternion to rotation matrix gives incorrect result!")
Quaternion p = q * r; // Quaternion multiplication and matrix multiplication have inverse orders
auto mp = mq * mr;
auto cp = p.RotationMatrix();
TEST_ASSERT_EQUAL(float, cp, "quaternions", mp, "matrices",
                  "Rotation matrix of multiplied quaternions is not the same as multiplied rotation matrices!")

auto pp = Transformations::RotateByQuaternion(point, p);
auto pm = mp * point;
TEST_ASSERT_EQUAL(float, pp, "quaternion", pm, "matrix",
                  "Matrix-point rotation and quaternion-point-rotation not equivalent!")

auto pr = Transformations::RotateByQuaternion(point, r);
auto prq = Transformations::RotateByQuaternion(pr, q);
TEST_ASSERT_EQUAL(float, pp, "direct", prq, "indirect",
                  "Quaternion rotation separation is not equivalent to in-one-go-rotation!")

END_TEST_CASE() // quaternion

BEGIN_TEST_CASE(maths)

RUN_SUB_CASE(vector)
RUN_SUB_CASE(matrix)
RUN_SUB_CASE(perspective)
RUN_SUB_CASE(quaternion)

END_TEST_CASE() // maths
} // namespace Test
