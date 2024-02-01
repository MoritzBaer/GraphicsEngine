#pragma once

#include "Window.h"
#include <vector>

namespace Engine
{
    class WindowManager
    {
    private:
        static inline WindowManager* windowManagerInstance = nullptr;

        std::vector<Window*> openWindows;

        inline Window* _CreateWindow(uint32_t width, uint32_t height, const char * title);
        inline void _DestroyWindow(Window * window);
        void _DeleteAllWindows();

        WindowManager();
    public:
        
        WindowManager(WindowManager &other) = delete;
        void operator=(const WindowManager &) = delete;

        static void Init();
        static void Cleanup();
        static Window* CreateWindow(uint32_t width, uint32_t height, const char * title);
        static void DestroyWindow(Window * window);
    };
    
} // namespace Engine::Graphics
