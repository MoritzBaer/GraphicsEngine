#pragma once

#include "Matrix.h"
#include "json-parsing.h"

namespace Engine::Maths {
class Quaternion {
  inline static Vector3 ProjectToVectorSpace(Vector3 const &v, Vector3 const &target) { return target * (v * target); }

  inline static Vector3 ProjectToOrthogonalVectorSpace(Vector3 const &v, Vector3 const &target) {
    return v - ProjectToVectorSpace(v, target);
  }

public:
  float w, x, y, z;
  Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
  Quaternion(Quaternion const &other) : w(other.w), x(other.x), y(other.y), z(other.z) {}
  Quaternion(Vector3 const &p) : w(0), x(p[X]), y(p[Y]), z(p[Z]) {}          // For rotating p (by calculating r p r*)
  Quaternion(float w, Vector3 const &p) : w(w), x(p[X]), y(p[Y]), z(p[Z]) {} // For rotating p (by calculating r p r*)
  Quaternion() : w(0), x(0), y(0), z(0) {}

  static inline Quaternion Identity() { return {1, 0, 0, 0}; }

  inline Quaternion operator*(Quaternion const &other) const;
  inline Quaternion operator+(Quaternion const &other) const {
    return {w + other.w, x + other.x, y + other.y, z + other.z};
  }

  inline Quaternion Conjugate() const { return {w, -x, -y, -z}; }
  inline Quaternion Normalized() const { return Quaternion(*this) / Vector4{w, x, y, z}.SqrMagnitude(); }
  inline Quaternion &Normalize();

  inline Quaternion operator*(float theta) const { return {w * theta, x * theta, y * theta, z * theta}; }
  inline Quaternion operator/(float theta) const { return {w / theta, x / theta, y / theta, z / theta}; }
  inline friend Quaternion operator*(float theta, Quaternion const &q) { return q * theta; }
  inline Quaternion &operator*=(Quaternion const &other);
  inline Quaternion &operator*=(float theta);
  inline Quaternion &operator/=(float theta);

  inline Vector3 xyz() const { return {x, y, z}; }

  inline static Quaternion RotateAroundAxis(Vector3 const &axis, float theta) {
    float cosTheta = cos(theta / 2);
    float sinTheta = sin(theta / 2);
    auto a = axis.Normalized();
    return {cosTheta, sinTheta * a[X], sinTheta * a[Y], sinTheta * a[Z]};
  }

  inline static Quaternion LookAt(Vector3 const &position, Vector3 const &target, Vector3 const &forward = {0, 1, 0},
                                  Vector3 const &up = {0, 0, 1}) {

    Vector3 newForward = (target - position).Normalized();

    // Project forward, newForward to vector space orthogonal to up
    Vector3 forwardFlat = ProjectToOrthogonalVectorSpace(forward, up);
    Vector3 newForwardFlat = ProjectToOrthogonalVectorSpace(newForward, up);

    // Calculate rotation perpendicular to up
    Vector3 rotationAxisFlat = forwardFlat.Cross(newForwardFlat);
    if (rotationAxisFlat.SqrMagnitude() < EPS) {
      rotationAxisFlat = up;
    } else {
      rotationAxisFlat.Normalize();
    }

    float dotFlat = newForwardFlat * forwardFlat;
    float angleFlat = acosf(dotFlat);

    Quaternion rotateFlat = RotateAroundAxis(rotationAxisFlat, angleFlat);

    // Calculate rotation towards up
    Vector3 rotationAxisLift = newForwardFlat.Cross(newForward);
    if (rotationAxisLift.SqrMagnitude() < EPS) {
      return rotateFlat;
    } else {
      rotationAxisLift.Normalize();
    }

    float dotLift = newForwardFlat * newForward;
    float angleLift = acosf(dotLift);

    return (RotateAroundAxis(rotationAxisLift, angleLift) * rotateFlat).Normalized();
  }

  // Conversions
  inline Vector3 EulerAngles() const;
  inline static Quaternion FromEulerAngles(Vector3 const &eulerAngles);
  inline Matrix3 RotationMatrix() const;
};

Quaternion Quaternion::operator*(Quaternion const &other) const {
  return {w * other.w - x * other.x - y * other.y - z * other.z, w * other.x + x * other.w + y * other.z - z * other.y,
          w * other.y - x * other.z + y * other.w + z * other.x, w * other.z + x * other.y - y * other.x + z * other.w};
}

Quaternion &Quaternion::Normalize() {
  *this /= Vector4{w, x, y, z}.Length();
  return *this;
}

inline Quaternion &Quaternion::operator*=(Quaternion const &other) {
  *this = *this * other;
  return *this;
}

inline Quaternion &Quaternion::operator*=(float theta) {
  w *= theta;
  x *= theta;
  y *= theta;
  z *= theta;
  return *this;
}

inline Quaternion &Quaternion::operator/=(float theta) {
  w /= theta;
  x /= theta;
  y /= theta;
  z /= theta;
  return *this;
}

inline Vector3 Quaternion::EulerAngles() const {
  return {atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z)),
          float(-PI / 2) + 2 * atan2(sqrt(1 + 2 * (w * y - x * z)), sqrt(1 - 2 * (w * y - x * z))),
          atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y))};
}

inline Quaternion Quaternion::FromEulerAngles(Vector3 const &eulerAngles) {
  float sinAlpha = sin(eulerAngles[0] / 2);
  float cosAlpha = cos(eulerAngles[0] / 2);
  float sinBeta = sin(eulerAngles[1] / 2);
  float cosBeta = cos(eulerAngles[1] / 2);
  float sinGamma = sin(eulerAngles[2] / 2);
  float cosGamma = cos(eulerAngles[2] / 2);

  return {cosGamma * cosBeta * cosAlpha + sinGamma * sinBeta * sinAlpha,
          sinGamma * cosBeta * cosAlpha - cosGamma * sinBeta * sinAlpha,
          cosGamma * sinBeta * cosAlpha + sinGamma * cosBeta * sinAlpha,
          cosGamma * cosBeta * sinAlpha - sinGamma * sinBeta * cosAlpha};
}

inline Matrix3 Quaternion::RotationMatrix() const {
  return Matrix3(1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w), 2 * (x * y + z * w),
                 1 - 2 * (x * x + z * z), 2 * (y * z - x * w), 2 * (x * z - y * w), 2 * (y * z + x * w),
                 1 - 2 * (x * x + y * y)) /
         sqrt(w * w + x * x + y * y + z * z);
}
} // namespace Engine::Maths

namespace std {
template <> struct formatter<Engine::Maths::Quaternion> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) const { return ctx.begin(); }

  template <typename FormatContext> auto format(Engine::Maths::Quaternion const &q, FormatContext &ctx) const {
    return ranges::copy(std::format("({:.3f}, {:.3f}, {:.3f}, {:.3f})", q.w, q.x, q.y, q.z), ctx.out()).out;
  }
};
} // namespace std

JSON(Engine::Maths::Quaternion, FIELDS(w, x, y, z))