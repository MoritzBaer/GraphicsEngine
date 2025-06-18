#pragma once

#include "Test.h"

#include "Debug/Logging.h"
#include "Graphics/Transform.h"

using namespace Engine::Maths;
using namespace Engine::Graphics;
using namespace Engine::Core;
using namespace Engine;

#define TEST_EPS                                                                                                       \
  0.00001 // Probably necessary because the matrix inversion has horrible condition. Might have to look into that
          // eventually.

namespace Test {
BEGIN_TEST_CASE(transform)

ECS ecs{};
ecs.RegisterComponent<Transform>();
ecs.RegisterComponent<HierarchyComponent>();
auto entity = ecs.CreateEntity();
auto transform = entity.AddComponent<Transform>();

Vector3 target = {-3, 4, -5};
Vector3 modelSpaceTarget = (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();
TEST_ASSERT_EQUAL(float, target, "world space", modelSpaceTarget, "model space",
                  "Model rotation not originally neutral!")

transform->LookAt(target);
auto M = transform->ModelToWorldMatrix();

Vector3 rotatedV =
    Transformations::RotateByQuaternion(Vector3(0, 0, (transform->position - target).Length()), transform->rotation);
TEST_ASSERT_EQUAL(float, rotatedV, "rotated", target, "target",
                  "LookAt quaternion does not rotate forward vector correctly!")

rotatedV = transform->rotation.RotationMatrix() * Vector3(0, 0, (transform->position - target).Length());
TEST_ASSERT_EQUAL(float, rotatedV, "rotated", target, "target",
                  "LookAt quaternion does not rotate forward vector correctly!")

auto Rd = transform->rotation.RotationMatrix();
auto Rt = transform->ModelToWorldMatrix();
for (int row = 0; row < 3; row++) {
  for (int col = 0; col < 3; col++) {
    TEST_ASSERT(Rd[row][col] == Rt[row][col],
                "Rotation matrix not transferred correctly: Rd[{},{}] = {}, Rt[{},{}] = {}", row, col, Rd[row][col],
                row, col, Rt[row][col])
  }
}

Vector3 nT = target.Normalized();
Vector3 tF = transform->Forward();
TEST_ASSERT_EQUAL(float, nT, "target", tF, "forward", "Transform forward does not point to target after LookAt!")

Vector3 transformedV =
    (transform->ModelToWorldMatrix() * Vector4(0, 0, (transform->position - target).Length(), 1)).xyz();
TEST_ASSERT_EQUAL(float, transformedV, "transformed", target, "target",
                  "Rotation quaternion does not translate correctly to model matrix!")

modelSpaceTarget = (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();
TEST_ASSERT(modelSpaceTarget[X] < TEST_EPS && modelSpaceTarget[Y] < TEST_EPS &&
                abs(modelSpaceTarget[Z] - target.Length()) < TEST_EPS,
            "LookAt does not look at target!")

transform->position = Vector3(0, 1, 0);

for (target = Vector3(0, 0, 2.6); target[Z] > -2.7; target -= Vector3(0, 0, 0.2)) {
  Engine::Debug::Logging::PrintMessage("Test", "Looking at moving target, current position: {}", target);

  transform->LookAt(target);
  M = transform->ModelToWorldMatrix();

  rotatedV =
      Transformations::RotateByQuaternion(Vector3(0, 0, (transform->position - target).Length()), transform->rotation) +
      transform->position;
  TEST_ASSERT_EQUAL(float, rotatedV, "rotated", target, "target",
                    "LookAt quaternion does not rotate forward vector correctly!")

  rotatedV = transform->rotation.RotationMatrix() * Vector3(0, 0, (transform->position - target).Length()) +
             transform->position;
  TEST_ASSERT_EQUAL(float, rotatedV, "rotated", target, "target",
                    "LookAt quaternion does not rotate forward vector correctly!")

  Rd = transform->rotation.RotationMatrix();
  Rt = transform->ModelToWorldMatrix();
  for (int row = 0; row < 3; row++) {
    for (int col = 0; col < 3; col++) {
      TEST_ASSERT(Rd[row][col] == Rt[row][col],
                  "Rotation matrix not transferred correctly: Rd[{},{}] = {}, Rt[{},{}] = {}", row, col, Rd[row][col],
                  row, col, Rt[row][col])
    }
  }

  nT = (target - transform->position).Normalized();
  tF = transform->Forward();
  TEST_ASSERT_EQUAL(float, nT, "target", tF, "forward", "Transform forward does not point to target after LookAt!")

  transformedV = (transform->ModelToWorldMatrix() * Vector4(0, 0, (transform->position - target).Length(), 1)).xyz();
  TEST_ASSERT_EQUAL(float, transformedV, "transformed", target, "target",
                    "Rotation quaternion does not translate correctly to model matrix!")

  modelSpaceTarget = (transform->WorldToModelMatrix() * Vector4(target[X], target[Y], target[Z], 1)).xyz();
  TEST_ASSERT(modelSpaceTarget[X] < TEST_EPS && modelSpaceTarget[Y] < TEST_EPS &&
                  abs(modelSpaceTarget[Z] - (transform->position - target).Length()) < TEST_EPS,
              "LookAt does not look at target!")
}

END_TEST_CASE()
} // namespace Test