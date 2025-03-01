#include "Core/Script.h"
#include "Core/Time.h"
#include "WindowedApplication.h"

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
    auto app = new GameApp<TestProject>("Test Project", {1600, 900});
    app->Run();
  } catch (std::exception &e) {
    ENGINE_ERROR("Exception: {}", e.what());
  }

  Engine::WindowManager::Cleanup();

  return 0;
}