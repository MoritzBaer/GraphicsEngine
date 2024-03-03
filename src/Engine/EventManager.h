#pragma once

#include "Window.h"
#include "Util/Macros.h"

namespace Engine
{
    class EventManager
    {
        _SINGLETON(EventManager)
        void _HandleWindowEvents(Window const * window);
    public:
        static void HandleWindowEvents(Window const * window);
    };
    
} // namespace Engine
