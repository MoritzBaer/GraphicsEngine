#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include "Util/Macros.h"
#include "Window.h"
#include <vector>
#include "backends/imgui_impl_vulkan.h"

namespace Engine::Graphics
{    
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR presentationSurface);
    
    class InstanceManager {
        _SINGLETON(InstanceManager, const char *, Window const *)

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

        static void FillImGUIInitInfo(ImGui_ImplVulkan_InitInfo & initInfo);

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
        static void CreateShaderModule(VkShaderModuleCreateInfo const * createInfo, VkShaderModule * shaderModule);
        static void CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo const * createInfo, VkDescriptorSetLayout * layout);
        static void CreateDescriptorPool(VkDescriptorPoolCreateInfo const * createInfo, VkDescriptorPool * descriptorPool);
        static void CreatePipelineLayout(VkPipelineLayoutCreateInfo const * createInfo, VkPipelineLayout * layout);
        static void CreateComputePipelines(std::vector<VkComputePipelineCreateInfo> const & createInfos, VkPipeline * pipelines);
        static inline void CreateComputePipeline(VkComputePipelineCreateInfo const & createInfo, VkPipeline * pipeline) { CreateComputePipelines({ createInfo }, pipeline); }
        static void CreateGraphicsPipelines(std::vector<VkGraphicsPipelineCreateInfo> const & createInfos, VkPipeline * pipelines);
        static inline void CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo const & createInfo, VkPipeline * pipeline) { CreateGraphicsPipelines({ createInfo }, pipeline); }

        // Destroy vulkan objects
        static inline void DestroySwapchain(VkSwapchainKHR const & swapchain) { vkDestroySwapchainKHR(instance->graphicsHandler, swapchain, nullptr); }
        static inline void DestroyImageView(VkImageView const & view) { vkDestroyImageView(instance->graphicsHandler, view, nullptr); }
        static inline void DestroyCommandPool(VkCommandPool const & pool) { vkDestroyCommandPool(instance->graphicsHandler, pool, nullptr); }
        static inline void DestroySemaphore(VkSemaphore const & semaphore) { vkDestroySemaphore(instance->graphicsHandler, semaphore, nullptr); }
        static inline void DestroyFence(VkFence const & fence) { vkDestroyFence(instance->graphicsHandler, fence, nullptr); }
        static inline void DestroyShaderModule(VkShaderModule const & shaderModule) { vkDestroyShaderModule(instance->graphicsHandler, shaderModule, nullptr); }
        static inline void DestroyDescriptorSetLayout(VkDescriptorSetLayout const & layout) { vkDestroyDescriptorSetLayout(instance->graphicsHandler, layout, nullptr); }
        static inline void DestroyDescriptorPool(VkDescriptorPool const & pool) { vkDestroyDescriptorPool(instance->graphicsHandler, pool, nullptr); }
        static inline void DestroyPipelineLayout(VkPipelineLayout const & layout) { vkDestroyPipelineLayout(instance->graphicsHandler, layout, nullptr); }
        static inline void DestroyPipeline(VkPipeline const & pipeline) { vkDestroyPipeline(instance->graphicsHandler, pipeline, nullptr); }

        // Allocate vulkan memory
        static void AllocateCommandBuffers(VkCommandBufferAllocateInfo const * allocInfo, VkCommandBuffer * commandBuffers);
        static void AllocateDescriptorSets(std::vector<VkDescriptorSetLayout> const & layouts, VkDescriptorPool const & descriptorPool, VkDescriptorSet * descriptorSets);
        static inline void AllocateDescriptorSets(VkDescriptorSetLayout const & layout, VkDescriptorPool const & descriptorPool, VkDescriptorSet * descriptorSet) { AllocateDescriptorSets(std::vector<VkDescriptorSetLayout>{ layout }, descriptorPool, descriptorSet); }

        // Free vulkan memory
        static inline void FreeCommandBuffers(VkCommandPool const & commandPool, VkCommandBuffer * buffers, uint32_t bufferCount = 1) { vkFreeCommandBuffers(instance->graphicsHandler, commandPool, bufferCount, buffers); }

        // Vulkan synchronization
        static void WaitForFences(VkFence const * fences, uint32_t fenceCount = 1, bool waitForAll = true, uint32_t timeout = 1000000000);
        static void ResetFences(VkFence const * fences, uint32_t fenceCount = 1);
        static inline void WaitUntilDeviceIdle() { vkDeviceWaitIdle(instance->graphicsHandler); }

        // Swapchain handling
        static uint32_t GetNextSwapchainImageIndex(VkSwapchainKHR const & swapchain, VkSemaphore const & semaphore = VK_NULL_HANDLE, VkFence const & fence = VK_NULL_HANDLE, uint32_t timeout = 1000000000);

        // Miscellaneous vulkan wrappers
        static inline void ClearDescriptorPool(VkDescriptorPool const & pool, VkDescriptorPoolResetFlags flags = 0) { vkResetDescriptorPool(instance->graphicsHandler, pool, flags); }
        static inline void UpdateDescriptorSets(std::vector<VkWriteDescriptorSet> const & descriptorSets) { vkUpdateDescriptorSets(instance->graphicsHandler, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr); }
        static inline void UpdateDescriptorSets(VkWriteDescriptorSet const & descriptorSet) { UpdateDescriptorSets(std::vector<VkWriteDescriptorSet>{ descriptorSet }); }
    };

} // namespace Engine::Graphics
