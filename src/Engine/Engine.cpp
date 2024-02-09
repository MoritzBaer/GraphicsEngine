#include "Engine.h"

bool quit = false;

void Engine::Init(const char * applicationName)
{
    WindowManager::Init(1600, 900, applicationName);
    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName);
    Graphics::Renderer::Init();
}

void Engine::RunMainLoop()
{
    while(!quit) {
        EventManager::HandleWindowEvents(WindowManager::GetMainWindow());
        if(WindowManager::GetMainWindow()->ShouldClose()) { Quit(); }
    }
}

void Engine::Cleanup()
{
    Graphics::Renderer::Cleanup();
    Graphics::InstanceManager::Cleanup();
    EventManager::Cleanup();
    WindowManager::Cleanup();
}

void Engine::Quit()
{
    quit = true;
}
