#include "EventManager.h"

#include "Debug/Logging.h"
#include "Util/Macros.h"
#include "WindowManager.h"

Engine::EventManager::EventManager() {}
Engine::EventManager::~EventManager() {}

void Engine::EventManager::Init()
{
    instance = new EventManager();
}

void Engine::EventManager::Cleanup()
{
    delete instance;
}

void Engine::EventManager::HandleWindowEvents()
{
    glfwPollEvents();
    WindowManager::CallAllCallbacks();
}
