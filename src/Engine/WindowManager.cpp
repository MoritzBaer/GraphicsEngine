#include "WindowManager.h"

#include "GLFW/glfw3.h"
#include "Debug/Logging.h"
#include "Util/Macros.h"

namespace Engine
{
    inline Window *WindowManager::_CreateWindow(uint32_t width, uint32_t height, const char *title)
    {
        openWindows.push_back(new Window(width, height, title));
        return openWindows.back();
    }

    inline void WindowManager::_DestroyWindow(Window *window)
    {
        openWindows.erase(std::find(openWindows.begin(), openWindows.end(), window));
        delete window;
    }

    void WindowManager::_DeleteAllWindows()
    {
        while(!openWindows.empty()) {
            Window * window = openWindows.back();
            openWindows.pop_back();
            delete window;
        }
    }

    WindowManager::WindowManager() : openWindows() { }
    WindowManager::~WindowManager() { }

    void WindowManager::Init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        instance = new WindowManager();
    }

    void WindowManager::Cleanup()
    {
        instance->_DeleteAllWindows();
        delete instance;
        glfwTerminate();
    }

    Window *WindowManager::CreateWindow(uint32_t width, uint32_t height, const char *title)
    {
        if(instance == nullptr) { 
            ENGINE_ERROR("Tried to create a window before initializing window manager!")
            return nullptr;
        } 
    
        return instance->_CreateWindow(width, height, title);
    }
    
    void WindowManager::DestroyWindow(Window *window)
    {
        if(instance == nullptr) { ENGINE_ERROR("Tried to destroy a window before initializing window manager!") } 

        instance->_DestroyWindow(window);
    }
} // namespace Engine::Graphics