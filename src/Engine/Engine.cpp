#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "AssetManager.h"
#include "Core/ECS.h"
#include "Core/SceneHierarchy.h"
#include "Core/Time.h"
#include "Debug/Logging.h"
#include "Debug/Profiling.h"
#include "Graphics/ImGUIManager.h"
#include "Util/DeletionQueue.h"
#include "Util/FileIO.h"
#include "imgui_internal.h"
#include <chrono>

#include "Editor/Display.h"

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

    Core::ECS::RegisterComponent<Graphics::Transform>();
    Core::ECS::RegisterComponent<Graphics::MeshRenderer>();
    Core::ECS::RegisterComponent<Graphics::Camera>();
    Core::ECS::RegisterComponent<Editor::Display>();

    mainCam = Core::Entity(Core::ECS::CreateEntity());
    mainCam.AddComponent<Graphics::Camera>();
    mainCam.AddComponent<Editor::Display>()->label = "Main camera";

    WindowManager::Init();
    mainWindow = WindowManager::CreateWindow(1600, 900, applicationName);
    mainWindow->SetCloseCallback([]() { Quit(); });
    mainWindow->SetMinimizeCallback([]() { render = false; });
    mainWindow->SetRestoreCallback([]() { render = true; });

    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName, mainWindow);
    Graphics::Renderer::Init(mainWindow->GetCanvasSize());
    Graphics::ImGUIManager::Init(mainWindow);

    auto monke2 = AssetManager::LoadPrefab("suzanne.obj");
    monke2.GetComponent<Editor::Display>()->label = "monke 2";
    auto monke21 = AssetManager::LoadPrefab("suzanne.obj");
    monke21.GetComponent<Editor::Display>()->label = "monke 2.1";
    monke21.GetComponent<Graphics::Transform>()->SetParent(monke2);
    Core::SceneHierarchy::BuildHierarchy();

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
      Graphics::ImGUIManager::BeginFrame();
      Graphics::Renderer::DrawFrame(mainCam.GetComponent<Graphics::Camera>());
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
