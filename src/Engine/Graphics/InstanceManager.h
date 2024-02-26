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
        static uint32_t GetPresentFamily();
        static void GetSwapchainImages(VkSwapchainKHR const & swapchain, std::vector<VkImage> & images);
        static inline void GetGraphicsQueue(VkQueue * queue) { vkGetDeviceQueue(instance->graphicsHandler, GetGraphicsFamily(), 0, queue); } 
        static inline void GetPresentQueue(VkQueue * queue) { vkGetDeviceQueue(instance->graphicsHandler, GetPresentFamily(), 0, queue); } 

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
        static void CreateSemaphore(VkSemaphoreCreateInfo const * createInfo, VkSemaphore * semaphore);
        static void CreateFence(VkFenceCreateInfo const * createInfo, VkFence * fence);

        // Destroy vulkan objects
        static inline void DestroySwapchain(VkSwapchainKHR & swapchain) { vkDestroySwapchainKHR(instance->graphicsHandler, swapchain, nullptr); }
        static inline void DestroyImageView(VkImageView & view) { vkDestroyImageView(instance->graphicsHandler, view, nullptr); }
        static inline void DestroyCommandPool(VkCommandPool & pool) { vkDestroyCommandPool(instance->graphicsHandler, pool, nullptr); }
        static inline void DestroySemaphore(VkSemaphore & semaphore) { vkDestroySemaphore(instance->graphicsHandler, semaphore, nullptr); }
        static inline void DestroyFence(VkFence & fence) { vkDestroyFence(instance->graphicsHandler, fence, nullptr); }

        // Allocate vulkan memory
        static void AllocateCommandBuffers(VkCommandBufferAllocateInfo const * allocInfo, VkCommandBuffer * commandBuffers);

        // Free vulkan memory
        static inline void FreeCommandBuffers(VkCommandPool const & commandPool, VkCommandBuffer * buffers, uint32_t bufferCount = 1) { vkFreeCommandBuffers(instance->graphicsHandler, commandPool, bufferCount, buffers); }

        // Vulkan synchronization
        static void WaitForFences(VkFence const * fences, uint32_t fenceCount = 1, bool waitForAll = true, uint32_t timeout = 1000000000);
        static void ResetFences(VkFence const * fences, uint32_t fenceCount = 1);

        // Swapchain handling
        static uint32_t GetNextSwapchainImageIndex(VkSwapchainKHR const & swapchain, VkSemaphore const & semaphore = VK_NULL_HANDLE, VkFence const & fence = VK_NULL_HANDLE, uint32_t timeout = 1000000000);
    };

} // namespace Engine::Graphics
