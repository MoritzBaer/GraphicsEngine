#include "EventManager.h"

#include "Debug/Logging.h"
#include "Macros.h"

Engine::EventManager::EventManager() {}

void Engine::EventManager::_HandleWindowEvents(Window const *window)
{
    glfwPollEvents();
    
}

void Engine::EventManager::Init()
{
    eventManagerInstance = new EventManager();
}

void Engine::EventManager::Cleanup()
{
    delete eventManagerInstance;
}

void Engine::EventManager::HandleWindowEvents(Window const *window)
{
    if(eventManagerInstance == nullptr) { 
        ENGINE_ERROR("Tried to handle window events before initializing EventManager!")
    }
    eventManagerInstance->_HandleWindowEvents(window);
}
