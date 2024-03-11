#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Util/DeletionQueue.h"
#include "Util/FileIO.h"
#include "AssetManager.h"
#include "Graphics/ImGUIManager.h"
#include "imgui_internal.h"

namespace Engine
{
    bool quit = false;
    bool render = true;

    Window * mainWindow;
} // namespace Engine

void Engine::Init(const char * applicationName)
{
    mainDeletionQueue.Create();

    Graphics::ShaderCompiler::Init();
    AssetManager::Init();
    WindowManager::Init();
    mainWindow = WindowManager::CreateWindow(1600, 900, applicationName);
    mainWindow->SetCloseCallback([](){ Quit(); });
    mainWindow->SetMinimizeCallback([](){ render = false; });
    mainWindow->SetRestoreCallback([](){ render = true; });

    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName, mainWindow);
    Graphics::Renderer::Init(mainWindow->GetCanvasSize());
    Graphics::ImGUIManager::Init(mainWindow);
}

void Engine::RunMainLoop()
{
    while(!quit) {
        EventManager::HandleWindowEvents();
        if (render) { 
            Graphics::ImGUIManager::BeginFrame();
            Graphics::Renderer::DrawFrame(); 
        }
        else { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
    }
}

void Engine::Cleanup()
{
    Graphics::InstanceManager::WaitUntilDeviceIdle();
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

void Engine::Quit()
{
    quit = true;
}
