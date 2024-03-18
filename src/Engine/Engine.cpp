#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Util/DeletionQueue.h"
#include "Util/FileIO.h"
#include "AssetManager.h"
#include "Graphics/ImGUIManager.h"
#include "imgui_internal.h"
#include "Debug/Logging.h"
#include "Core/Time.h"
#include "Core/ECS.h"
#include "Debug/Profiling.h"
#include <chrono>

namespace Engine
{
    bool quit = false;
    bool render = true;

    Window * mainWindow;
} // namespace Engine

void Engine::Init(const char * applicationName)
{
    BEGIN_PROFILE_SESSION()
    {
        PROFILE_FUNCTION()

        mainDeletionQueue.Create();

        Graphics::ShaderCompiler::Init();
        AssetManager::Init();
        Core::ECS::Init();

        Core::ECS::RegisterComponent<Graphics::Transform>();
        Core::ECS::RegisterComponent<Graphics::MeshRenderer>();

        WindowManager::Init();
        mainWindow = WindowManager::CreateWindow(1600, 900, applicationName);
        mainWindow->SetCloseCallback([](){ Quit(); });
        mainWindow->SetMinimizeCallback([](){ render = false; });
        mainWindow->SetRestoreCallback([](){ render = true; });

        EventManager::Init();
        Graphics::InstanceManager::Init(applicationName, mainWindow);
        Graphics::Renderer::Init(mainWindow->GetCanvasSize());
        Graphics::ImGUIManager::Init(mainWindow);
        Time::Update();
    }
    WRITE_PROFILE_SESSION("Init")
}

void Engine::RunMainLoop()
{
    BEGIN_PROFILE_SESSION()
    while(!quit) {
        PROFILE_SCOPE("Main loop iteration")
        EventManager::HandleWindowEvents();
        Time::Update();
        if (render) { 
            Graphics::ImGUIManager::BeginFrame();
            Graphics::Renderer::DrawFrame(); 
        }
        else { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
    }
    WRITE_PROFILE_SESSION("Loop")
}

void Engine::Cleanup()
{
    BEGIN_PROFILE_SESSION()
    {
        PROFILE_FUNCTION()
        Graphics::InstanceManager::WaitUntilDeviceIdle();
        mainDeletionQueue.Flush();
        Graphics::ImGUIManager::Cleanup();
        Graphics::Renderer::Cleanup();
        Graphics::InstanceManager::Cleanup();
        EventManager::Cleanup();
        WindowManager::Cleanup();
        Core::ECS::Cleanup();
        AssetManager::Cleanup();
        Graphics::ShaderCompiler::Cleanup();

        mainDeletionQueue.Destroy();
    }
    WRITE_PROFILE_SESSION("Cleanup")
}

void Engine::Quit()
{
    quit = true;
}
