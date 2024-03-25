#pragma once

#include "Core/ECS.h"
#include "Maths/Transformations.h"

using namespace Engine::Maths;

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Transform) {
  Vector3 position;
  Quaternion rotation;
  Vector3 scale;

  Transform *parent;
  std::vector<Transform *> children;

  ENGINE_COMPONENT_CONSTRUCTOR(Transform), position(Vector3::Zero()), rotation(Quaternion::Identity()),
      scale(Vector3::One()), parent(nullptr), children() {}

  // Scale is not adjusted as by stacking scales and rotation, shearing is possible (which cannot be represented as a
  // Vector3)
  inline void SetParent(Transform * newParent);
  inline void SetParent(Core::Entity const &newParent);

  inline Matrix4 ModelToParentMatrix() const;
  inline Matrix4 ParentToModelMatrix() const { return ModelToParentMatrix().Inverse(); }
  inline Matrix4 ModelToWorldMatrix() const;
  inline Matrix4 WorldToModelMatrix() const { return ModelToWorldMatrix().Inverse(); }

  inline Vector3 WorldPosition() const {
    return (ModelToWorldMatrix() * Vector4{position[X], position[Y], position[Z], 1}).xyz();
  }
  inline Quaternion WorldRotation() const;
};

inline void Transform::SetParent(Transform *newParent) {
  // Remove from old parent
  position = WorldPosition();
  rotation = WorldRotation();
  if (parent) {
    parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
  }

  // Add to new parent
  parent = newParent;
  newParent->children.push_back(this);
  position = (parent->WorldToModelMatrix() * Vector4{position[X], position[Y], position[Z], 1}).xyz();
  rotation *= parent->WorldRotation().Conjugate();
}

inline void Transform::SetParent(Core::Entity const &newParent) {
  ENGINE_ASSERT(newParent.HasComponent<Transform>(), "Tried to assign a transformless entity as parent!")
  SetParent(newParent.GetComponent<Transform>());
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
