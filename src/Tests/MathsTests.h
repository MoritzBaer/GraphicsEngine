#pragma once

#include "Test.h"

#include "Maths/Transformations.h"

#include "Debug/Logging.h"

using namespace Engine::Maths;
using namespace Engine;

TEST_CASE("Vectors work correctly") {
  Maths::Vector4 v = {20, 34, 95, 35};

  SUB_GROUP("Internal representation is correct") {
    float explicit_v[] = {20, 34, 95, 35};
    VERIFY_MEM_EQUAL(v, explicit_v, float);
  }

  SUB_GROUP("Length and normalization work correctly") {
    VERIFY(v.Length() == 108.655418641f);

    Vector4 oldV = v;
    v.Normalize();
    VERIFY(std::abs(v.Length() - 1) < EPS);
    VERIFY(v == oldV / oldV.Length());
  }

  SUB_GROUP("Cross product works correctly") {
    Vector3 a{1, 2, 3};
    Vector3 b{3, 4, 5};
    Vector3 axb = a.Cross(b);
    Vector3 correct_axb = {-2, 4, -2};

    VERIFY(a.Cross(b) == correct_axb);
  }
}

TEST_CASE("Accessing vector entries works correctly") {
  Vector<10> v{20, 34, 95, 35, 98, 84, 20, 85, 18, 70};

  SUB_GROUP("Retrieving first four entries gives correct result") {
    Vector4 v2 = v.xyzw();
    VERIFY_MEM_EQUAL(v2, v, float);
  }

  SUB_GROUP("Subtracting vector obtained through accessor gives correct result") {
    Vector4 test = (v.x() - v.Entries<0, 2, 7, 4>());
    Vector4 intendedResult = {0, -75, -65, -78};
    VERIFY(test == intendedResult);
  }

  SUB_GROUP("Assignment gives correct value") {
    Vector4 v2 = {1, 2, 3, 4};
    Vector4 oldV2 = v2;
    v.xyzw() = v2;
    VERIFY_MEM_EQUAL(v, v2, float);
    VERIFY(v2 == oldV2);
  }

  SUB_GROUP("Assignment-increment gives correct result") {
    Vector2 test = (v.xy() += v.x());
    Vector2 expected = {40, 54};
    VERIFY_MEM_EQUAL(test, v, float);
    VERIFY(test == expected);

    Vector4 test2 = v.Entries<0, 2, 7, 4>();
    Vector2 compounded = test2.xz() += test2.yw();
    Vector2 expectedCompund = {135, 183};
    Vector2 xz = test2.xz();
    VERIFY_MEM_EQUAL(xz, compounded, float);
    VERIFY(compounded == expectedCompund);
  }
}

TEST_CASE("Matrices work correctly") {
  SUB_GROUP("Internal representation is correct") {
    Matrix3 M = Matrix3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    float explicit_M[] = {1, 4, 7, 2, 5, 8, 3, 6, 9};
    VERIFY_MEM_EQUAL(M, explicit_M, float)

    Matrix4 m1 = Matrix4(8.96836, 2.10230, 1.89949, 4.60040, 8.39039, 5.83538, 7.79529, 6.32591, 4.75992, 5.49344,
                         0.39212, 0.78537, 6.66451, 8.18015, 3.31983, 4.87304);
    // Matrices should be stored in column-major form but row-major constructors are more legible
    float explicit_m1[] = {8.96836, 8.39039, 4.75992, 6.66451, 2.10230, 5.83538, 5.49344, 8.18015,
                           1.89949, 7.79529, 0.39212, 3.31983, 4.60040, 6.32591, 0.78537, 4.87304};
    VERIFY_MEM_EQUAL(m1, explicit_m1, float)
  }

  SUB_GROUP("Accessing matrix entries works correctly") {
    Matrix3 M = Matrix3(1, 2, 3, 4, 5, 6, 7, 8, 9);

    Matrix3 const &m = M;
    float m_12 = m[1][2];
    VERIFY(m_12 == 8);
  }

  // TODO: Deconstruct multiplication in VERIFY
  SUB_GROUP("Matrix multiplication gives correct result") {
    MatrixNM<2, 3> A = MatrixNM<2, 3>(1, 2, 3, 3, 2, 1);
    Matrix3 B{4, 1, 0, 0, 3, 0, 2, 1, 2};

    MatrixNM<2, 3> C = A * B;
    MatrixNM<2, 3> expected{10, 10, 6, 14, 10, 2};
    VERIFY(C == expected);

    Matrix4 m1 = Matrix4(8.96836, 2.10230, 1.89949, 4.60040, 8.39039, 5.83538, 7.79529, 6.32591, 4.75992, 5.49344,
                         0.39212, 0.78537, 6.66451, 8.18015, 3.31983, 4.87304);
    Matrix4 m2{5.03589, 6.78238, 7.17521, 3.66256, 8.01326, 4.52237, 6.94845, 0.59203,
               5.60207, 2.59969, 7.80068, 7.32192, 0.61612, 5.25223, 6.61894, 3.31925};

    Matrix4 m3 = m1 * m2;
    Matrix4 correct_m3 = {75.4854253307,  99.4346479979,  124.2246780198, 63.2695727914, 136.5807781554, 136.7870325182,
                          203.4290379255, 112.2587933441, 70.671362396,   62.2712887803, 80.5814383006,  26.1636845213,
                          120.7117055058, 116.4197600212, 162.8099129266, 69.7344296437};
    VERIFY(m3 == correct_m3);
  }

  SUB_GROUP("Matrix-vector multiplication gives correct result") {
    Matrix4 M{1, 3, 2, 4, 0, 1, 1, 3, 2, 0, 0, 2, 1, 2, 4, 1};
    Vector4 x{4, 2, 3, 0};
    Vector4 y = M * x;
    Vector4 expected = Vector4{16, 5, 8, 20};
    VERIFY(y == expected);

    Matrix4 M_{5.03589, 6.78238, 7.17521, 3.66256, 8.01326, 4.52237, 6.94845, 0.59203,
               5.60207, 2.59969, 7.80068, 7.32192, 0.61612, 5.25223, 6.61894, 3.31925};
    Maths::Vector4 x_{9.85868, 1.70382, 6.27017, 6.29855};
    Maths::Vector4 y_ = M_ * x_;
    Maths::Vector4 expected_ = {129.2617865, 134.0023638, 154.6874882, 77.43132555};
    VERIFY(y_ == expected_);
  }

  SUB_GROUP("Inverse matrix is correct") {
    Matrix2 M = Matrix2(1, 1, 2, 3);
    Matrix2 M_Inv = M.Inverse();
    Matrix2 M_Exp = Matrix2(3, -1, -2, 1);
    VERIFY(M_Inv == M_Exp);

    Matrix4 m = Matrix4(6.01589, 0.79945, 3.03696, 7.89833, 2.89674, 6.73960, 5.22906, 1.90390, 4.23525, 3.65487,
                        7.92008, 6.07612, 0.21322, 7.01907, 0.17421, 7.19930);
    Matrix4 m_Inv = m.Inverse();
    Matrix4 m_Exp =
        Matrix4(0.22590520535147221064, 0.21439106151673295909, -0.22566118257648591511, -0.11408172710246326558,
                -0.0048146176943861615186, 0.15176869603639391364, -0.099435098775642683715, 0.0490679109584360566,
                -0.11926301806954870517, -0.067545549318803498536, 0.21735238026807963992, -0.034736779955989534575,
                0.00088945329587463363156, -0.15268435171168592384, 0.098369624539275549675, 0.095282166115857150355);
    VERIFY(m_Inv == m_Exp);
  }

  // TODO: Add test for larger matrix
  SUB_GROUP("Determinant is correct") {
    float a = 2, b = 3, c = -2, d = 5;
    Matrix2 M = Matrix2(a, b, c, d);
    float determinant = M.Determinant();
    float expected = a * d - b * c;
  }
}

TEST_CASE("Perspective matrix behaves correctly") {
  auto P = Transformations::Perspective(0.01f, 100.0f, 60.0f, 16.0f / 9.0f);

  SUB_GROUP("Point in front of camera is transformed correctly") {
    Vector4 v = {0, 11, 0, 1};
    Vector4 vp = P * v;
    Vector3 vh = vp.xyz() / vp.w();

    VERIFY(vh[X] == 0);
    VERIFY(vh[Y] == 0);
    VERIFY(vh[Z] > 0);
    VERIFY(vh[Z] < 1);

    Vector4 u = {0.01, 1.99, 0.01, 1};
    Vector4 up = P * u;
    Vector3 uh = up.xyz() / up.w();

    VERIFY(uh[X] > 0);
    VERIFY(uh[X] < 1);
    VERIFY(uh[Y] < 0);
    VERIFY(uh[Y] > -1);
    VERIFY(uh[Z] > 0);
    VERIFY(uh[Z] < 1);
  }

  SUB_GROUP("Point in front and right of camera is transformed correctly") {
    Vector4 v = {1, 10, 0, 1};
    Vector4 vp = P * v;
    Vector3 vh = vp.xyz() / vp.w();
    VERIFY(vh[X] > 0);
    VERIFY(vh[X] < 1);
    VERIFY(vh[Y] == 0);
    VERIFY(vh[Z] > 0);
    VERIFY(vh[Z] < 1);
  }

  SUB_GROUP("Point in front of and above camera is transformed correctly") {
    Vector4 v = {0, 10, 1, 1};
    Vector4 vp = P * v;
    Vector3 vh = vp.xyz() / vp.w();

    VERIFY(vh[X] == 0);
    VERIFY(vh[Y] < 0);
    VERIFY(vh[Y] > -1);
    VERIFY(vh[Z] > 0);
    VERIFY(vh[Z] < 1);
  }
}

TEST_CASE("Permutation iterator works correctly") {
  constexpr size_t const n = 6;

  Engine::Maths::PermutationIterator<uint32_t, n> sigma{};
  while (true) {
    size_t numInversions = 0;
    for (size_t i = 0; i < n - 1; i++) {
      for (size_t j = i + 1; j < n; j++) {
        numInversions += sigma[i] > sigma[j];
      }
    }

    VERIFY(sigma.NumberOfInversions() == numInversions);
    if (!sigma.HasValidSuccessor())
      break;
    sigma++;
  }
}

TEST_CASE("Quaternions work correctly") {

  Vector3 axis1{0.73059, 1.19045, 0.55717};
  Vector3 axis2{0.73404, 0.95724, 0.69343};
  Vector3 point{3.71460, 12.14657, 3.76972};
  float angle = 13.0125f;
  Quaternion q = Transformations::RotateAroundAxis(axis1, angle);
  Quaternion r = Transformations::RotateAroundAxis(axis2, 0.32f);

  auto mq = Transformations::RodriguesRotation(axis1, angle);
  auto mr = Transformations::RodriguesRotation(axis2, 0.32f);

  Quaternion p =
      q *
      r; // Quaternion multiplication and matrix multiplication have inverse orders - Then why are the orders the same??

  SUB_GROUP("Conversion to rotation matrix gives correct result") {
    auto cq = q.RotationMatrix();
    VERIFY(mq == cq);
    auto cr = r.RotationMatrix();
    VERIFY(mr == cr);
  }

  SUB_GROUP("Quaternion multiplication gives multiplied rotation matrix") {
    auto mp = mq * mr;
    auto cp = p.RotationMatrix();
    VERIFY(cp == mp);
  }

  SUB_GROUP("Quaternion-point rotation is equivalent to matrix-point rotation") {
    auto pp = Transformations::RotateByQuaternion(point, p);
    auto pm = p.RotationMatrix() * point;
    VERIFY(pp == pm);
  }

  SUB_GROUP("Separated rotation is equivalent to in-one-go rotation") {
    auto pp = Transformations::RotateByQuaternion(point, p);
    auto pr = Transformations::RotateByQuaternion(point, r);
    auto prq = Transformations::RotateByQuaternion(pr, q);
    VERIFY(pp == prq);
  }
}