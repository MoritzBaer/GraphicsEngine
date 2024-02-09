#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include "../Macros.h"
#include "../Window.h"
#include <vector>

namespace Engine::Graphics
{    
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR presentationSurface);
    
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
        static inline SwapchainSupportDetails GetSwapchainSupport() { return QuerySwapchainSupport(instance->gpu, instance->surface); }
        static void CreateSwapchain(
            VkSurfaceFormatKHR surfaceFormat, 
            VkPresentModeKHR presentMode, 
            VkExtent2D extent, 
            uint32_t imageCount, 
            VkSwapchainKHR oldSwapchain, 
            VkSwapchainKHR * swapchain);
        static void GetSwapchainImages(VkSwapchainKHR &swapchain, std::vector<VkImage> & images);

        static inline void DestroySwapchain(VkSwapchainKHR swapchain) { vkDestroySwapchainKHR(instance->graphicsHandler, swapchain, nullptr); }
    };

} // namespace Engine::Graphics
