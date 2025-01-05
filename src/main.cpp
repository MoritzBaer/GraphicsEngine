#include "Core/Script.h"
#include "Core/Time.h"
#include "Editor/Display.h"
#include "Editor/Editor.h"

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
  TestProject() : Game("Test Project") {}

  void Init() override {
    Game::Init();
    activeScene = assetManager.LoadAsset<Core::Scene *>("testscene");
    // auto secondSpeeder = activeScene->InstantiateEntity(assetManager.LoadAsset<Core::Entity>("speeder"));
    // auto transform = secondSpeeder.GetComponent<Engine::Graphics::Transform>();
    // auto meshRenderer = transform->children[0]->entity.GetComponent<Engine::Graphics::MeshRenderer>();
    activeScene->mainCamera.GetComponent<Engine::Graphics::Transform>()->LookAt({0, 0, 0});
  }
};

int main() {

  Engine::WindowManager::Init();

  auto gameSize = sizeof(TestProject);

  TestProject game{};
  Editor::Editor editor{&game};
  try {
    editor.Init();

    while (editor.IsRunning())
      editor.CalculateFrame();
  } catch (std::exception &e) {
    ENGINE_ERROR("Exception: {}", e.what());
  }

  Engine::WindowManager::Cleanup();

  return 0;
}