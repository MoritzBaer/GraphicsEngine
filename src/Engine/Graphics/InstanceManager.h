#pragma once

#include "vulkan/vulkan.h"

namespace Engine::Graphics
{
    class InstanceManager {
        static InstanceManager* instanceManagerInstance;

        VkInstance instance;

        InstanceManager();

    public:
        InstanceManager(InstanceManager &other) = delete;
        void operator=(const InstanceManager &) = delete;
    };

} // namespace Engine::Graphics
