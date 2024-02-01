#include "WindowManager.h"

#include "glfw3.h"
#include "Debug/Logging.h"

namespace Engine
{
    inline Window *WindowManager::_CreateWindow(uint32_t width, uint32_t height, const char *title)
    {
        return new Window(width, height, title);
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

    WindowManager::WindowManager() : openWindows() {}

    void WindowManager::Init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        windowManagerInstance = new WindowManager();
    }

    void WindowManager::Cleanup()
    {
        windowManagerInstance->_DeleteAllWindows();
        delete windowManagerInstance;
        glfwTerminate();
    }

    Window *WindowManager::CreateWindow(uint32_t width, uint32_t height, const char *title)
    {
        if(windowManagerInstance == nullptr) { 
            Debug::Logging::PrintError("Tried to create a window before initializing window manager!", "Engine"); 
            return nullptr;
        } 
    
        return windowManagerInstance->_CreateWindow(width, height, title);
    }
    
    void WindowManager::DestroyWindow(Window *window)
    {
        if(windowManagerInstance == nullptr) { Debug::Logging::PrintError("Tried to destroy a window before initializing window manager!", "Engine"); } 

        windowManagerInstance->_DestroyWindow(window);
    }
} // namespace Engine::Graphics