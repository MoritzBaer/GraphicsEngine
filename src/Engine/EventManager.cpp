#include "EventManager.h"

#include "Debug/Logging.h"

Engine::EventManager::EventManager() {}

void Engine::EventManager::_HandleWindowEvents(Window const *window)
{
    glfwPollEvents();
    
}

void Engine::EventManager::Init()
{
    eventManagerInstance = new EventManager();
}

void Engine::EventManager::Close()
{
    delete eventManagerInstance;
}

void Engine::EventManager::HandleWindowEvents(Window const *window)
{
    if(eventManagerInstance == nullptr) { 
        Debug::Logging::PrintError("Tried to handle window events before initializing EventManager!", "Engine");
    }
    eventManagerInstance->_HandleWindowEvents(window);
}
