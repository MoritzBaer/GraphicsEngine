#pragma once

#include "Util/Macros.h"

namespace Engine
{
    class EventManager
    {
        _SINGLETON(EventManager)
    public:
        static void HandleWindowEvents();
    };
    
} // namespace Engine
