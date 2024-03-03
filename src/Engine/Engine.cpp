#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Util/DeletionQueue.h"
#include "Util/FileIO.h"
#include "AssetManager.h"

bool quit = false;

void Engine::Init(const char * applicationName)
{
    mainDeletionQueue.Create();

    Graphics::ShaderCompiler::Init();
    AssetManager::Init();
    WindowManager::Init(1600, 900, applicationName);
    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName);
    Graphics::Renderer::Init(WindowManager::GetMainWindow()->GetCanvasSize());
}

void Engine::RunMainLoop()
{
    while(!quit) {
        EventManager::HandleWindowEvents(WindowManager::GetMainWindow());
        if(WindowManager::GetMainWindow()->ShouldClose()) { Quit(); }
        Graphics::Renderer::DrawFrame();
    }
}

void Engine::Cleanup()
{
    Graphics::InstanceManager::WaitUntilDeviceIdle();
    mainDeletionQueue.Flush();
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
