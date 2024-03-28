#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "AssetManager.h"
#include "Core/ECS.h"
#include "Core/SceneHierarchy.h"
#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "EventManager.h"
#include "Graphics/ImGUIManager.h"
#include "Util/DeletionQueue.h"
#include "Util/FileIO.h"
#include "WindowManager.h"
#include "imgui_internal.h"
#include <chrono>

#include "Editor/Display.h"
#include "Editor/EntityDetails.h"

namespace Engine {
bool quit = false;
bool render = true;

Window *mainWindow;

Core::Entity mainCam;
} // namespace Engine

void Engine::Init(const char *applicationName) {
  BEGIN_PROFILE_SESSION() {
    PROFILE_FUNCTION()

    mainDeletionQueue.Create();

    Graphics::ShaderCompiler::Init();
    AssetManager::Init();
    Core::ECS::Init();

    Core::ECS::RegisterComponent<Editor::Display>(); // Must be at the top so name is displayed above other components
    Core::ECS::RegisterComponent<Graphics::Transform>();
    Core::ECS::RegisterComponent<Graphics::MeshRenderer>();
    Core::ECS::RegisterComponent<Graphics::Camera>();

    mainCam = Core::Entity(Core::ECS::CreateEntity());
    mainCam.AddComponent<Graphics::Camera>();
    mainCam.AddComponent<Editor::Display>()->AssignLabel("Main camera");

    WindowManager::Init();
    mainWindow = WindowManager::CreateWindow(1600, 900, applicationName);
    mainWindow->SetCloseCallback([]() { Quit(); });
    mainWindow->SetMinimizeCallback([]() { render = false; });
    mainWindow->SetRestoreCallback([]() { render = true; });

    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName, mainWindow);
    Graphics::Renderer::Init(mainWindow->GetCanvasSize());
    Graphics::ImGUIManager::Init(mainWindow);

    Time::Update();
  }
  WRITE_PROFILE_SESSION("Init")
}

void Engine::RunMainLoop() {
  BEGIN_PROFILE_SESSION()
  while (!quit) {
    PROFILE_SCOPE("Main loop iteration")
    EventManager::HandleWindowEvents();
    Time::Update();
    if (render) {
      auto renderersWithTransforms = Core::ECS::FilterEntities<Graphics::MeshRenderer, Graphics::Transform>();
      for (auto const &[_, transform] : renderersWithTransforms) {
        transform->rotation = Maths::Transformations::RotateAroundAxis(Maths::Vector3(0, 1, 0), Time::time * 0.2f);
        transform->position.y() = std::sin(Time::time) * 0.1f;
      }
      std::vector<Graphics::MeshRenderer const *> meshRenderers(renderersWithTransforms.size());
      std::transform(renderersWithTransforms.begin(), renderersWithTransforms.end(), meshRenderers.begin(),
                     [](auto const &t) { return std::get<0>(t); });
      Graphics::ImGUIManager::BeginFrame();
      Graphics::Renderer::DrawFrame(mainCam.GetComponent<Graphics::Camera>(), meshRenderers);
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  WRITE_PROFILE_SESSION("Loop")
}

void Engine::Cleanup() {
  BEGIN_PROFILE_SESSION() {
    PROFILE_FUNCTION()
    Graphics::InstanceManager::WaitUntilDeviceIdle();
    Core::ECS::Cleanup();
    mainDeletionQueue.Flush();
    Graphics::ImGUIManager::Cleanup();
    Graphics::Renderer::Cleanup();
    Graphics::InstanceManager::Cleanup();
    EventManager::Cleanup();
    WindowManager::Cleanup();
    AssetManager::Cleanup();
    Graphics::ShaderCompiler::Cleanup();

    mainDeletionQueue.Destroy();
  }
  WRITE_PROFILE_SESSION("Cleanup")
}

void Engine::Quit() { quit = true; }
