#pragma once

#include "vulkan/vulkan.h"

namespace Engine::Graphics
{
    class InstanceManager {
        static inline InstanceManager* instanceManagerInstance = nullptr;

        VkInstance vulkanInstance;
        VkDebugUtilsMessengerEXT debugMessenger;

        InstanceManager(const char * applicationName);
        ~InstanceManager();

        void CreateInstance(const char * applicationName);
#ifndef NDEBUG
        void EnableValidationLayers();
        void SetupDebugMessenger();
#endif
    public:
        InstanceManager(InstanceManager &other) = delete;
        void operator=(const InstanceManager &) = delete;

        static void Init(const char * applicationName);
        static void Cleanup();
    };

} // namespace Engine::Graphics
