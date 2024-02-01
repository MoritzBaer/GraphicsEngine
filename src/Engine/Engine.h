#pragma once

#include "WindowManager.h"
#include "Debug/Logging.h"
#include "EventManager.h"

namespace Engine
{
    void Init();

    void RunMainLoop();

    void Cleanup();

    void Quit();
} // namespace Engine
