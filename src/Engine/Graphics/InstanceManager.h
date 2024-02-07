#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include "../Macros.h"
#include "../Window.h"

namespace Engine::Graphics
{
    class InstanceManager {
        _SINGLETON(InstanceManager, const char *)

        VkInstance vulkanInstance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;
        VkPhysicalDevice gpu;
        VkDevice graphicsHandler;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        void CreateInstance(const char * applicationName);
#ifndef NDEBUG
        void EnableValidationLayers();
        void SetupDebugMessenger();
#endif
        void PickPhysicalDevice();
        void CreateLogicalDevice();

    public:
    };

} // namespace Engine::Graphics
