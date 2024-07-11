#pragma once

#include "Matrix.h"
#include "json-parsing.h"

namespace Engine::Maths {
class Quaternion {
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

  inline static Quaternion LookAt(Vector3 const &position, Vector3 const &target, Vector3 const &up) {
    Vector3 F = (target - position).Normalized(); // lookAt
    Vector3 R = F.Cross(up).Normalized();         // sideaxis
    Vector3 U = R.Cross(F);                       // rotatedup

    // note that R needed to be re-normalized
    // since F and worldUp are not necessary perpendicular
    // so must remove the sin(angle) factor of the cross-product
    // same not true for U because dot(R, F) = 0

    // adapted source
    float trace = R.x() + U.y() + F.z();
    if (trace > 0.0) {
      float s = 0.5f / sqrt(trace + 1.0);
      return {0.25f / s, (U.z() - F.y()) * s, (F.x() - R.z()) * s, (R.y() - U.x()) * s};
    } else {
      if (R.x() > U.y() && R.x() > F.z()) {
        float s = 2.0f * sqrt(1.0f + R.x() - U.y() - F.z());
        return {(U.z() - F.y()) / s, 0.25f * s, (U.x() + R.y()) / s, (F.x() + R.z()) / s};
      } else if (U.y() > F.z()) {
        float s = 2.0f * sqrt(1.0f + U.y() - R.x() - F.z());
        return {(F.x() - R.z()) / s, (U.x() + R.y()) / s, 0.25f * s, (F.y() + U.z()) / s};
      } else {
        float s = 2.0f * sqrt(1.0f + F.z() - R.x() - U.y());
        return {(R.y() - U.x()) / s, (F.x() + R.z()) / s, (F.y() + U.z()) / s, 0.25f * s};
      }
    }
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
  float sqrLen = Vector4{w, x, y, z}.SqrMagnitude();
  *this /= sqrLen;
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
  return {atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y)),
          float(-PI / 2) + 2 * atan2(sqrt(1 + 2 * (w * y - x * z)), sqrt(1 - 2 * (w * y - x * z))),
          atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z))};
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
  return Matrix3{w * w + x * x - y * y - z * z, 2 * (x * y + w * z),           2 * (x * z - w * y),
                 2 * (x * y - w * z),           w * w - x * x + y * y - z * z, 2 * (w * x + y * z),
                 2 * (w * y + x * z),           2 * (y * z - w * x),           w * w - x * x - y * y + z * z};
}
} // namespace Engine::Maths

OBJECT_PARSER(Engine::Maths::Quaternion, FIELD_PARSER(w) FIELD_PARSER(x) FIELD_PARSER(y) FIELD_PARSER(z))
OBJECT_SERIALIZER(Engine::Maths::Quaternion,
                  FIELD_SERIALIZER(w) FIELD_SERIALIZER(x) FIELD_SERIALIZER(y) FIELD_SERIALIZER(z))