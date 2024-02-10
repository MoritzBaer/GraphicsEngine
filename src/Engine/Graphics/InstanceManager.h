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
        static uint32_t GetGraphicsFamily();
        static void GetSwapchainImages(VkSwapchainKHR const & swapchain, std::vector<VkImage> & images);

        // Create vulkan objects
        static void CreateSwapchain(
            VkSurfaceFormatKHR const & surfaceFormat, 
            VkPresentModeKHR const & presentMode, 
            VkExtent2D const & extent, 
            uint32_t const & imageCount, 
            VkSwapchainKHR const & oldSwapchain, 
            VkSwapchainKHR * swapchain);    // Maybe allow caller to choose every value but surface at some point
        static void CreateImageView(VkImageViewCreateInfo const * createInfo, VkImageView * view);
        static void CreateCommandPool(VkCommandPoolCreateInfo const * createInfo, VkCommandPool * commandPool);

        // Destroy vulkan objects
        static inline void DestroySwapchain(VkSwapchainKHR & swapchain) { vkDestroySwapchainKHR(instance->graphicsHandler, swapchain, nullptr); }
        static inline void DestroyImageView(VkImageView & view) { vkDestroyImageView(instance->graphicsHandler, view, nullptr); }
        static inline void DestroyCommandPool(VkCommandPool & pool) { vkDestroyCommandPool(instance->graphicsHandler, pool, nullptr); }

        // Allocate vulkan memory
        static void AllocateCommandBuffers(VkCommandBufferAllocateInfo const * allocInfo, VkCommandBuffer * commandBuffers);

        // Free vulkan memory
        static inline void FreeCommandBuffers(VkCommandPool const & commandPool, VkCommandBuffer * buffers, uint32_t bufferCount = 1) { vkFreeCommandBuffers(instance->graphicsHandler, commandPool, bufferCount, buffers); }
    };

} // namespace Engine::Graphics
