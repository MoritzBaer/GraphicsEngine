#include "Editor/Display.h"
#include "Editor/Editor.h"

struct TestProject : public Game {
  TestProject() : Game("Test Project") {}

  void SpecializedInit() override {
    mainCam = ecs.CreateEntity();
    mainCam.AddComponent<Engine::Graphics::Camera>();
    mainCam.AddComponent<Editor::Display>()->AssignLabel("Main camera");
    auto camTransform = mainCam.AddComponent<Engine::Graphics::Transform>();
    camTransform->position = {0, 0, 5};
    camTransform->LookAt({0, 0, 0});

    assetManager.LoadAsset<Engine::Core::Entity>("speeder");

    assetManager.LoadAsset<Engine::Core::Entity>("cube");
    assetManager.LoadAsset<Engine::Core::Entity>("cube");
    assetManager.LoadAsset<Engine::Core::Entity>("cube")
        .Duplicate()
        .GetComponent<Engine::Graphics::Transform>()
        ->position = {-3, 0, -4};
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