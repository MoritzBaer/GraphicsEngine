#pragma once

#include "vulkan/vulkan.h"
#include <optional>

namespace Engine::Graphics
{
    class InstanceManager {
        static inline InstanceManager* instanceManagerInstance = nullptr;

        VkInstance vulkanInstance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice gpu;
        VkDevice graphicsHandler;
        VkQueue graphicsQueue;

        InstanceManager(const char * applicationName);
        ~InstanceManager();

        void CreateInstance(const char * applicationName);
#ifndef NDEBUG
        void EnableValidationLayers();
        void SetupDebugMessenger();
#endif
        void PickPhysicalDevice();
        void CreateLogicalDevice();
    public:
        InstanceManager(InstanceManager &other) = delete;
        void operator=(const InstanceManager &) = delete;

        static void Init(const char * applicationName);
        static void Cleanup();
    };

} // namespace Engine::Graphics
