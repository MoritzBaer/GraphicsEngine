#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include "../Macros.h"

namespace Engine::Graphics
{
    class InstanceManager {
        _SINGLETON(InstanceManager, const char *)

        VkInstance vulkanInstance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice gpu;
        VkDevice graphicsHandler;
        VkQueue graphicsQueue;

        void CreateInstance(const char * applicationName);
#ifndef NDEBUG
        void EnableValidationLayers();
        void SetupDebugMessenger();
#endif
        void PickPhysicalDevice();
        void CreateLogicalDevice();
    };

} // namespace Engine::Graphics
