#pragma once

#include <string>

#include "Core/HierarchyComponent.h"
#include "Debug/Logging.h"
#include "Maths/Transformations.h"
#include "json-parsing.h"

using namespace Engine::Maths;

namespace Engine::Graphics {
struct Transform : public Core::HierarchicalComponent<Transform> {
  Vector3 position;
  Quaternion rotation;
  Vector3 scale;

  Transform *parent;

  Transform(Core::Entity entity)
      : Core::HierarchicalComponent<Transform>(entity), position(Vector3::Zero()), rotation(Quaternion::Identity()),
        scale(Vector3::One()), parent(nullptr) {}

  inline void SetParent(Transform *newParent, bool recalculateTransform = true);

  inline void LookAt(Vector3 const &target, Vector3 const &up) { rotation = Quaternion::LookAt(position, target, up); }
  // TODO: Use Transform::Up()
  inline void LookAt(Vector3 const &target) { LookAt(target, {0, 1, 0}); }
  inline void Translate(Vector3 const &translation) { position += translation; }

  inline Matrix4 ModelToParentMatrix() const;
  inline Matrix4 ParentToModelMatrix() const { return ModelToParentMatrix().Inverse(); }
  inline Matrix4 ModelToWorldMatrix() const;
  inline Matrix4 WorldToModelMatrix() const { return ModelToWorldMatrix().Inverse(); }

  inline Vector3 WorldPosition() const {
    return (ModelToWorldMatrix() * Vector4{position[X], position[Y], position[Z], 1}).xyz();
  }
  inline Quaternion WorldRotation() const;

  inline bool HasInactiveParent() const {
    if (parent) {
      return !parent->entity.IsActive() || parent->HasInactiveParent();
    } else {
      return false;
    }
  }

  inline void CopyFrom(Transform const &other) override {
    position = other.position;
    rotation = other.rotation;
    scale = other.scale;
  }

  inline void OnHierarchyChange() override { SetParent(hierarchy->parent->entity.GetComponent<Transform>(), true); }
};

// Scale is not adjusted as by stacking scales and rotation, shearing is possible (which cannot be represented as a
// Vector3)
inline void Transform::SetParent(Transform *newParent, bool recaltulateTransform) {
  // Remove from old parent
  if (recaltulateTransform) {
    position = WorldPosition();
    rotation = WorldRotation();
  }

  // Add to new parent
  parent = newParent;
  if (recaltulateTransform) {
    position = (parent->WorldToModelMatrix() * Vector4{position[X], position[Y], position[Z], 1}).xyz();
    rotation *= parent->WorldRotation().Conjugate();
  }
}

Matrix4 Transform::ModelToParentMatrix() const {
  {
    Matrix3 R = rotation.RotationMatrix();
    return Matrix4{scale[X] * R[0][0],
                   scale[Y] * R[0][1],
                   scale[Z] * R[0][2],
                   position[X],

                   scale[X] * R[1][0],
                   scale[Y] * R[1][1],
                   scale[Z] * R[1][2],
                   position[Y],

                   scale[X] * R[2][0],
                   scale[Y] * R[2][1],
                   scale[Z] * R[2][2],
                   position[Z],

                   0,
                   0,
                   0,
                   1};
  }
}

Matrix4 Transform::ModelToWorldMatrix() const {
  if (parent) {
    return parent->ModelToWorldMatrix() * ModelToParentMatrix();
  } else {
    return ModelToParentMatrix();
  }
}

Quaternion Transform::WorldRotation() const {
  if (parent) {
    return parent->WorldRotation() * rotation;
  } else {
    return rotation;
  }
}

} // namespace Engine::Graphics