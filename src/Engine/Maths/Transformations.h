#pragma once

#include "Matrix.h"
#include "Quaternion.h"

namespace Engine::Maths::Transformations {

// Transform matrices
template <typename T>
inline MatrixT<4, 4, T> LookAt(VectorT<3, T> const &eye, VectorT<3, T> const &target, VectorT<3, T> const &up);

template <typename T> inline MatrixT<3, 3, T> RodriguesRotation(VectorT<3, T> const &axis, T theta);

template <typename T> inline MatrixT<4, 4, T> Rotate(VectorT<3, T> const &axis, T theta); // Rotate around origin

template <typename T>
inline MatrixT<4, 4, T> Rotate(VectorT<3, T> const &axis, VectorT<3, T> const &center, T theta); // Rotate around center

template <typename T> inline MatrixT<4, 4, T> Perspective(T near, T far, T fov, T aspectRatio);

// Quaternion operations

inline Quaternion RotateAroundAxis(Vector3 const &axis, float theta);

inline Vector3 RotateByQuaternion(Vector3 const &point, Quaternion const &rotation);

// +-------------------+
// |  IMPLEMENTATIONS  |
// +-------------------+

inline Quaternion RotateAroundAxis(Vector3 const &axis, float theta) {
  float cosTheta = cos(theta / 2);
  float sinTheta = sin(theta / 2);
  auto a = axis.Normalized();
  return {cosTheta, sinTheta * a[X], sinTheta * a[Y], sinTheta * a[Z]};
}

template <typename T> inline MatrixT<4, 4, T> Perspective(T near, T far, T fov, T aspectRatio) {
  T s = T(1) / tan(PI * fov / T(360));
  return {s / aspectRatio,
          0,
          0,
          0,
          0,
          -s,
          0,
          0,
          0,
          0,
          -(far + near) / (far - near),
          -1,
          0,
          0,
          -2 * far * near / (far - near),
          0};
}

template <typename T> MatrixT<3, 3, T> RodriguesRotation(VectorT<3, T> const &axis, T theta) {
  T sinTheta = sin(theta);
  T cosTheta = cos(theta);
  auto a = axis.Normalized();
  return MatrixT<3, 3, T>{
      cosTheta + a[X] * a[X] * (T(1) - cosTheta),         a[Z] * sinTheta + a[X] * a[Y] * (T(1) - cosTheta),
      -a[Y] * sinTheta + a[X] * a[Z] * (T(1) - cosTheta),

      -a[Z] * sinTheta + a[Y] * a[X] * (T(1) - cosTheta), cosTheta + a[Y] * a[Y] * (T(1) - cosTheta),
      a[X] * sinTheta + a[Y] * a[Z] * (T(1) - cosTheta),

      a[Y] * sinTheta + a[Z] * a[X] * (T(1) - cosTheta),  -a[X] * sinTheta + a[Z] * a[Y] * (T(1) - cosTheta),
      cosTheta + a[Z] * a[Z] * (T(1) - cosTheta)};
}

template <typename T> MatrixT<4, 4, T> Rotate(VectorT<3, T> const &axis, T theta) {
  auto R = RodriguesRotation(axis, theta);
  // auto R = RotateAroundAxis(axis, theta).RotationMatrix();
  return MatrixT<4, 4, T>{R[0][0], R[0][1], R[0][2], 0, R[1][0], R[1][1], R[1][2], 0,
                          R[2][0], R[2][1], R[2][2], 0, 0,       0,       0,       1};
}

template <typename T> MatrixT<4, 4, T> Rotate(VectorT<3, T> const &axis, VectorT<3, T> const &center, T theta) {
  auto R = RodriguesRotation(axis, theta);
  auto Rp = center - R * center;
  return MatrixT<4, 4, T>{R[0][0], R[0][1], R[0][2], 0, R[1][0], R[1][1], R[1][2], 0,
                          R[2][0], R[2][1], R[2][2], 0, Rp[0],   Rp[1],   Rp[2],   1};
}

template <typename T>
MatrixT<4, 4, T> LookAt(VectorT<3, T> const &eye, VectorT<3, T> const &target, VectorT<3, T> const &up) {
  const VectorT<3, T> f = (target - eye).Normalized();
  const VectorT<3, T> r = f.Cross(up).Normalized();
  const VectorT<3, T> u = r.Cross(f).Normalized();

  return MatrixT<4, 4, T>{r[X], u[X], -f[X], 0, r[Y],       u[Y],       -f[Y],   0,
                          r[Z], u[Z], -f[Z], 0, -(r * eye), -(u * eye), f * eye, 1};
}

inline Vector3 RotateByQuaternion(Vector3 const &point, Quaternion const &rotation) {
  // FIXME: Maybe broken, figure out why
  Vector3 t = 2 * rotation.XYZ().Cross(point);
  // return 2.0f * (point * u) * u + s * s - (point * u) * point + 2.0f * s * u.Cross(point);
  return point + rotation.w * t + rotation.XYZ().Cross(t);
}

} // namespace Engine::Maths::Transformations
