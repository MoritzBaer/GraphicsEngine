#pragma once

#include "Window.h"
#include <vector>
#include "Macros.h"

namespace Engine
{
    class WindowManager
    {
        _SINGLETON(WindowManager, uint32_t width, uint32_t height, const char *title)

        std::vector<Window*> openWindows;

        inline Window* _CreateWindow(uint32_t width, uint32_t height, const char * title);
        inline void _DestroyWindow(Window * window);
        void _DeleteAllWindows();

    public:
    
        static Window * CreateWindow(uint32_t width, uint32_t height, const char * title);
        static void DestroyWindow(Window * window);
        inline static Window const * GetMainWindow() { return instance->openWindows[0]; }
    };
    
} // namespace Engine::Graphics
