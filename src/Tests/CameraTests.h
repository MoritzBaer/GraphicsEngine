#include "Core/ECS.h"
#include "Graphics/Camera.h"
#include "Maths/Matrix.h"
#include "Maths/Transformations.h"
#include "Test.h"

TEST_CASE("Changing aspect ratio yields same result as new perspective matrix") {
  float fov = 45, near = 0.01f, far = 100.0f;
  float initialAR = 1.7777778f, newAR = 1.4f;

  Engine::Core::ECS ecs{};
  Engine::Core::Entity entity = ecs.CreateEntity();
  ecs.RegisterComponent<Engine::Graphics::Camera>();
  Engine::Graphics::Camera *cam = entity.AddComponent<Engine::Graphics::Camera>();
  cam->projection = Maths::Transformations::Perspective(near, far, fov, initialAR);
  cam->aspectRatio = initialAR;
  cam->SetAspectRatio(newAR);

  Engine::Maths::Matrix4 expectedProjection = Maths::Transformations::Perspective(near, far, fov, newAR);
  VERIFY_MEM_EQUAL(cam->projection, expectedProjection, float)
}