#pragma once

#include "WindowManager.h"
#include "Debug/Logging.h"
#include "EventManager.h"
#include "Graphics/InstanceManager.h"
#include "Graphics/Renderer.h"

namespace Engine
{
    void Init(const char * applicationName);

    void RunMainLoop();

    void Cleanup();

    void Quit();
} // namespace Engine
