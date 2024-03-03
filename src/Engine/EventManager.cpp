#include "EventManager.h"

#include "Debug/Logging.h"
#include "Util/Macros.h"

Engine::EventManager::EventManager() {}
Engine::EventManager::~EventManager() {}

void Engine::EventManager::_HandleWindowEvents(Window const *window)
{
    glfwPollEvents();
    
}

void Engine::EventManager::Init()
{
    instance = new EventManager();
}

void Engine::EventManager::Cleanup()
{
    delete instance;
}

void Engine::EventManager::HandleWindowEvents(Window const *window)
{
    if(instance == nullptr) { 
        ENGINE_ERROR("Tried to handle window events before initializing EventManager!")
    }
    instance->_HandleWindowEvents(window);
}
