#include "Engine.h"

Engine::Window * mainWindow;
bool quit = false;

void Engine::Init()
{
    WindowManager::Init();
    EventManager::Init();
    mainWindow = WindowManager::CreateWindow(1600, 900, "Main window");
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
    WindowManager::Cleanup();
}

void Engine::Quit()
{
    quit = true;
}
