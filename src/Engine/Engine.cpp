#include "Engine.h"

#include "DeletionQueue.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

bool quit = false;

void Engine::Init(const char * applicationName)
{
    mainDeletionQueue.Create();
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
    mainDeletionQueue.Destroy();
}

void Engine::Quit()
{
    quit = true;
}
