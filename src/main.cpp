#include "Editor/Display.h"
#include "Editor/Editor.h"

struct TestProject : public Game {
  TestProject() : Game("Test Project") {}

  void Init() override {
    Game::Init();

    auto speeder = assetManager.LoadAsset<Engine::Core::Entity>("speeder");
    ENGINE_ASSERT(speeder.IsActive(), "Speeder is not active!");

    assetManager.LoadAsset<Engine::Core::Entity>("cube");
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