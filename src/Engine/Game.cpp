#include "Game.h"

#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Editor/Display.h"
#include "Graphics/Camera.h"
#include "Graphics/MeshRenderer.h"
#include "Graphics/Transform.h"

Game::Game(const char *name)
    : mainWindow(Engine::WindowManager::CreateWindow(1600, 900, name)), instanceManager(name, mainWindow),
      assetManager(this), shaderCompiler(instanceManager), ecs(), memoryAllocator(),
      gpuObjectManager(instanceManager, memoryAllocator), renderer({1600, 900}, instanceManager, gpuObjectManager),
      sceneHierarchy(ecs), render(true), imGuiManager(mainWindow, renderer.GetSwapchainFormat(), instanceManager) {

  BEGIN_PROFILE_SESSION() {
    PROFILE_FUNCTION()

    mainDeletionQueue.Create();

    // TODO: Remove once scene loading no longer happens in constructor
    ecs.RegisterComponent<Engine::Editor::Display>(); // Must be at the top so name is displayed above other
                                                      // components
    ecs.RegisterComponent<Engine::Graphics::Transform>();
    ecs.RegisterComponent<Engine::Graphics::MeshRenderer>();
    ecs.RegisterComponent<Engine::Graphics::Camera>();

    mainCam = ecs.CreateEntity();
    mainCam.AddComponent<Engine::Graphics::Camera>();
    mainCam.AddComponent<Engine::Editor::Display>()->AssignLabel("Main camera");
    auto camTransform = mainCam.AddComponent<Engine::Graphics::Transform>();
    camTransform->position = {0, 0, 5};
    camTransform->LookAt({0, 0, 0});

    // mainWindow->SetCloseCallback([]() { Quit(); });
    mainWindow->SetMinimizeCallback([this]() { render = false; });
    mainWindow->SetRestoreCallback([this]() { render = true; });
    mainWindow->SetResizeCallback(
        [this](Engine::Maths::Dimension2 newWindowSize) { renderer.SetWindowSize(newWindowSize); });

    assetManager.InitStandins();

    Engine::Time::Update();
  }
  WRITE_PROFILE_SESSION("Init")
}

void Game::CalculateFrame() {
  {
    PROFILE_FUNCTION()

    Engine::Time::Update();

    Engine::WindowManager::HandleEventsOnAllWindows();

    if (render) {
      auto renderersWithTransforms = ecs.FilterEntities<Engine::Graphics::MeshRenderer, Engine::Graphics::Transform>();
      for (auto &[_, transform] : renderersWithTransforms) {
        if (transform->parent)
          transform = transform->parent->entity.GetComponent<Engine::Graphics::Transform>();
        transform->rotation = (Engine::Maths::Transformations::RotateAroundAxis(Engine::Maths::Vector3(0, 1, 0),
                                                                                Engine::Time::deltaTime * 0.2f) *
                               transform->rotation)
                                  .Normalized();
        transform->position.y() = std::sin(Engine::Time::time) * 0.1f;
      }
      std::vector<Engine::Graphics::MeshRenderer const *> meshRenderers(renderersWithTransforms.size());
      std::transform(renderersWithTransforms.begin(), renderersWithTransforms.end(), meshRenderers.begin(),
                     [](auto const &t) { return std::get<0>(t); });
      imGuiManager.BeginFrame();
      renderer.DrawFrame(mainCam.GetComponent<Engine::Graphics::Camera>(),
                         {mainCam.GetComponent<Engine::Graphics::Transform>()->position,
                          Engine::Maths::Vector3(0, -5, -1.5).Normalized(),
                          {1.2f, 0.8f, 0.6f}},
                         meshRenderers);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

Game::~Game() {
  BEGIN_PROFILE_SESSION()
  PROFILE_FUNCTION()

  instanceManager.WaitUntilDeviceIdle();
  ecs.~ECS();
  mainDeletionQueue.Flush();
  imGuiManager.~ImGUIManager();
  renderer.~Renderer();
  assetManager.~AssetManager();
  instanceManager.~InstanceManager();
  WRITE_PROFILE_SESSION("Cleanup")
}
