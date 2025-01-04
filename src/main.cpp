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
  float bobbingAmplitude;

  BobbyScript(Core::Entity entity, float bobbingAmplitude) : Core::Script(entity), bobbingAmplitude(bobbingAmplitude) {}

  void OnCreate() override { transform = entity.GetComponent<Engine::Graphics::Transform>(); }
  void OnUpdate(float deltaTime) override { transform->position.y() = std::sin(Engine::Time::time) * bobbingAmplitude; }

  Script *Clone() override { return new BobbyScript(entity, bobbingAmplitude); }
};

struct TestProject : public Game {
  TestProject() : Game("Test Project") {}

  void Init() override {
    Game::Init();

    auto speederScripts = assetManager.LoadAsset<Engine::Core::Entity>("speeder").AddComponent<Core::ScriptComponent>();
    speederScripts->InstantiateScript<SpinnyScript>();
    speederScripts->InstantiateScript<BobbyScript>(0.1f);

    assetManager.LoadAsset<Engine::Core::Entity>("cube")
        .AddComponent<Core::ScriptComponent>()
        ->InstantiateScript<BobbyScript>(1.0f);
    assetManager.LoadAsset<Engine::Core::Entity>("cube").GetComponent<Engine::Graphics::Transform>()->position = {-3, 0,
                                                                                                                  -4};
  }
};

int main() {

  Engine::WindowManager::Init();

  auto gameSize = sizeof(TestProject);

  Game *game = new TestProject();
  Editor::Editor editor(game);
  editor.Init();

  while (editor.IsRunning())
    editor.CalculateFrame();

  delete game;

  Engine::WindowManager::Cleanup();

  return 0;
}