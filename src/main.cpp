#include "Core/Script.h"
#include "Core/Time.h"
#include "Editor/Display.h"
#include "Editor/Editor.h"
#include "WindowedApplication.h"

struct SpinnyScript : public Core::Script {
  Engine::Graphics::Transform *transform;
  float rotationSpeed = 1.0f;

  SpinnyScript(Core::Entity entity) : Core::Script(entity) {}
  void OnCreate() override { transform = entity.GetComponent<Engine::Graphics::Transform>(); }

  void OnUpdate(float deltaTime) override {
    transform->rotation = (Engine::Maths::Transformations::RotateAroundAxis(Engine::Maths::Vector3(0, 1, 0),
                                                                            Engine::Time::deltaTime * 0.2f) *
                           transform->rotation)
                              .Normalized();
  }

  Script *Clone() override { return new SpinnyScript(entity); }
};

struct BobbyScript : public Core::Script {
  Engine::Graphics::Transform *transform;
  float initialY;
  float bobbingAmplitude;

  BobbyScript(Core::Entity entity, float bobbingAmplitude) : Core::Script(entity), bobbingAmplitude(bobbingAmplitude) {}

  void OnStart() override { initialY = transform->position.y(); }
  void OnCreate() override { transform = entity.GetComponent<Engine::Graphics::Transform>(); }
  void OnUpdate(float deltaTime) override {
    transform->position.y() = initialY + std::sin(Engine::Time::time) * bobbingAmplitude;
  }

  Script *Clone() override { return new BobbyScript(entity, bobbingAmplitude); }
};

struct TestProject : public Game {
  TestProject(Engine::Graphics::VulkanSuite
#ifdef NDEBUG
              const
#endif
                  *vulkan)
      : Game("Test Project", vulkan) {
  }

  void Init() override {
    Game::Init();
    activeScene = assetManager.LoadAsset<Core::Scene *>("testscene");
    activeScene->mainCamera.GetComponent<Engine::Graphics::Transform>()->LookAt({0, 0, 0});
  }
};

int main() {

  Engine::WindowManager::Init();

  try {
    auto app = new Editor::EditorApp<TestProject>("Test Project", {1600, 900});
    app->Run();
  } catch (std::exception &e) {
    ENGINE_ERROR("Exception: {}", e.what());
  }

  Engine::WindowManager::Cleanup();

  return 0;
}