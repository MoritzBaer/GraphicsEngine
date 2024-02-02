#include "Engine.h"

Engine::Window * mainWindow;
bool quit = false;

void Engine::Init(const char * applicationName)
{
    WindowManager::Init();
    EventManager::Init();
    Graphics::InstanceManager::Init(applicationName);
    mainWindow = WindowManager::CreateWindow(1600, 900, applicationName);
}

void Engine::RunMainLoop()
{
    while(!quit) {
        EventManager::HandleWindowEvents(mainWindow);
        if(mainWindow->ShouldClose()) { Quit(); }
    }
}

void Engine::Cleanup()
{
    Graphics::InstanceManager::Cleanup();
    EventManager::Cleanup();
    WindowManager::Cleanup();
}

void Engine::Quit()
{
    quit = true;
}
