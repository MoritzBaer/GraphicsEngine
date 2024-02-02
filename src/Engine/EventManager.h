#pragma once

#include "Window.h"

namespace Engine
{
    class EventManager
    {
    private:
        EventManager();
        void _HandleWindowEvents(Window const * window);
        static inline EventManager * eventManagerInstance = nullptr;
    public:
        EventManager(EventManager &other) = delete;
        void operator=(const EventManager &) = delete;

        static void Init();
        static void Cleanup();
        static void HandleWindowEvents(Window const * window);
    };
    
} // namespace Engine
