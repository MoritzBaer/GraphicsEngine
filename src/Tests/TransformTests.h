#pragma once

#include "Test.h"

#include "Debug/Logging.h"
#include "Graphics/Transform.h"

using namespace Engine::Maths;
using namespace Engine::Graphics;
using namespace Engine::Core;
using namespace Engine;

TEST_CASE("Transform component works correctly") {

  ECS ecs{};
  ecs.RegisterComponent<Transform>();
  ecs.RegisterComponent<HierarchyComponent>();
  auto entity = ecs.CreateEntity();
  auto transform = entity.AddComponent<Transform>();

  Vector3 target = {-3, 4, -5};
  Vector3 modelSpaceTarget = (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();

  SUB_GROUP("Model rotation is identity on creation") { VERIFY(target == modelSpaceTarget); }

  SUB_GROUP("LookAt quaternion rotates forward vector correctly") {
    transform->LookAt(target);
    auto M = transform->ModelToWorldMatrix();

    Vector3 rotatedV = Transformations::RotateByQuaternion(Vector3(0, 0, (transform->position - target).Length()),
                                                           transform->rotation);
    VERIFY(rotatedV == target);

    rotatedV = transform->rotation.RotationMatrix() * Vector3(0, 0, (transform->position - target).Length());
    VERIFY(rotatedV == target);
  }

  SUB_GROUP("Rotation matrix is included correctly in model matrix") {
    transform->LookAt(target);

    auto Rd = transform->rotation.RotationMatrix();
    auto Rt = transform->ModelToWorldMatrix();
    for (int row = 0; row < 3; row++) {
      for (int col = 0; col < 3; col++) {
        VERIFY(Rd[row][col] == Rt[row][col]);
      }
    }
  }

  SUB_GROUP("Transform forward points at target after LookAt") {
    transform->LookAt(target);

    Vector3 nT = target.Normalized();
    Vector3 tF = transform->Forward();
    VERIFY(nT == tF);

    Vector3 transformedV =
        (transform->ModelToWorldMatrix() * Vector4(0, 0, (transform->position - target).Length(), 1)).xyz();
    VERIFY(transformedV == target);

    modelSpaceTarget = (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();
    VERIFY(Vector3(modelSpaceTarget[X], modelSpaceTarget[Y], abs(modelSpaceTarget[Z] - target.Length())) ==
            Vector3::Zero);
  }

  SUB_GROUP("Follow moving target") {

    transform->position = Vector3(0, 1, 0);

    for (target = Vector3(0, 0, 2.6); target[Z] > -2.7; target -= Vector3(0, 0, 0.2)) {
      testLogger.PrintMessage("Looking at moving target, current position: {}", target);

      transform->LookAt(target);
      auto M = transform->ModelToWorldMatrix();

      auto rotatedV = Transformations::RotateByQuaternion(Vector3(0, 0, (transform->position - target).Length()),
                                                          transform->rotation) +
                      transform->position;
      VERIFY(rotatedV == target);

      rotatedV = transform->rotation.RotationMatrix() * Vector3(0, 0, (transform->position - target).Length()) +
                 transform->position;

      VERIFY(rotatedV == target);

      auto const Rd = transform->rotation.RotationMatrix();
      auto const Rt = transform->ModelToWorldMatrix();
      for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
          VERIFY(Rd[row][col] == Rt[row][col]);
        }
      }

      auto const nT = (target - transform->position).Normalized();
      auto const tF = transform->Forward();
      VERIFY(nT == tF);

      Vector3 const transformedV =
          (transform->ModelToWorldMatrix() * Vector4(0, 0, (transform->position - target).Length(), 1)).xyz();
      VERIFY(transformedV == target);

      auto const modelSpaceTarget =
          (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();
      VERIFY(Vector3(modelSpaceTarget[X], modelSpaceTarget[Y], abs(modelSpaceTarget[Z] - target.Length())) ==
              Vector3::Zero);
    }
  }
}