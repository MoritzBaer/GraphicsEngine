#pragma once

#include <string>

#include "Core/ECS.h"
#include "Editor/Publishable.h"
#include "Maths/Transformations.h"
#include "json-parsing.h"

using namespace Engine::Maths;

namespace Engine::Graphics {
ENGINE_COMPONENT_DECLARATION(Transform), public Editor::Publishable {
private:
  // TODO: Change publication so only the displayed entity's transform has to hold this data
  // Probably should work quite nicely as a trait
  struct {
    Vector3 eulerAngles;
    Vector3 oldEulerAngles;

    void UpdateRotation(Quaternion &output) {
      if (eulerAngles != oldEulerAngles) {
        output = Quaternion::FromEulerAngles(eulerAngles);
      } else {
        eulerAngles = output.EulerAngles();
      }
      oldEulerAngles = eulerAngles;
    }
  } rotationPublication;

public:
  Vector3 position;
  Quaternion rotation;
  Vector3 scale;

  Transform *parent;
  std::vector<Transform *> children;

  ENGINE_COMPONENT_CONSTRUCTOR(Transform), position(Vector3::Zero()), rotation(Quaternion::Identity()),
      scale(Vector3::One()), parent(nullptr), children(), Publishable("Transform") {}

  // Scale is not adjusted as by stacking scales and rotation, shearing is possible (which cannot be represented as a
  // Vector3)
  inline void SetParent(Transform * newParent, bool recalculateTransform = true);
  inline void SetParent(Core::Entity const &newParent, bool recalculateTransform = true);

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

  inline std::vector<Editor::Publication> GetPublications() override {
    rotationPublication.UpdateRotation(rotation);
    return {PUBLISH(position), PUBLISH_RANGE(rotationPublication.eulerAngles, -(float)PI, (float)PI, 0.001f),
            PUBLISH_RANGE(scale, 0.001f, 10000.0f, 0.01f * scale.Length())};
  }

  inline void CopyFrom(Core::Component const *other) override {
    if (auto otherTransform = dynamic_cast<Transform const *>(other)) {
      position = otherTransform->position;
      rotation = otherTransform->rotation;
      scale = otherTransform->scale;
    } else {
      ENGINE_ERROR("Tried to copy Transform from different type!");
    }
  }
};

inline void Transform::SetParent(Transform *newParent, bool recaltulateTransform) {
  // Remove from old parent
  if (recaltulateTransform) {
    position = WorldPosition();
    rotation = WorldRotation();
  }
  if (parent) {
    parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
  }

  // Add to new parent
  parent = newParent;
  newParent->children.push_back(this);
  if (recaltulateTransform) {
    position = (parent->WorldToModelMatrix() * Vector4{position[X], position[Y], position[Z], 1}).xyz();
    rotation *= parent->WorldRotation().Conjugate();
  }
}

inline void Transform::SetParent(Core::Entity const &newParent, bool recalculateTransform) {
  ENGINE_ASSERT(newParent.HasComponent<Transform>(), "Tried to assign a transformless entity as parent!")
  SetParent(newParent.GetComponent<Transform>(), recalculateTransform);
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