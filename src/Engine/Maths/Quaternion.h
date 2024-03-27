#pragma once

#include "Matrix.h"
#include "Util/Serializable.h"

namespace Engine::Maths {
class Quaternion : public Util::Serializable {
public:
  float w, x, y, z;
  Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
  Quaternion(Quaternion const &other) : w(other.w), x(other.x), y(other.y), z(other.z) {}
  Quaternion(Vector3 const &p) : w(0), x(p[X]), y(p[Y]), z(p[Z]) {} // For rotating p (by calculating r p r*)
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

  // Conversions
  inline Vector3 EulerAngles() const;
  inline static Quaternion FromEulerAngles(Vector3 const &eulerAngles);
  inline Matrix3 RotationMatrix() const;

  inline void Serialize(std::stringstream &targetStream) const override {
    targetStream << "{w: " << w << ", x: " << x << ", y: " << y << ", z: " << z << "}";
  }
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
