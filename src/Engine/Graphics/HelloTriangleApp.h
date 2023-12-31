#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include "vulkan/vulkan.h"
#include <vector>
#include <optional>

class HelloTriangleApp
{
private:

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateVKInstance();
    bool CheckValidationLayerSupport();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    std::vector<const char*> GetRequiredExtensions();
    void SetupDebugMessenger();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice const &device) const;
    uint32_t PhysicalDeviceScore(VkPhysicalDevice const &device) const;
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice const &device) const;
    SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice const &device) const;
    VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR const & capabilities) const;
    VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> & availableModes) const;
    VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
    void CreateSwapchain();
    void CreateImageViews();
    void CreateGraphicsPipeline();
    VkShaderModule CreateShaderModule(std::vector<char> const & code) const;
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateVertexBuffer();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void Draw();
    void CreateSyncObjects();
    void CleanupSwapchain();
    void RecreateSwapchain();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    GLFWwindow *window;

    const uint16_t WINDOW_WIDTH = 1600;
    const uint16_t WINDOW_HEIGHT = 900;

    VkInstance vulkanInstance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalGPU = VK_NULL_HANDLE;
    VkDevice graphicsHandler;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    const std::vector<const char*> requiredValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static void FramebufferResizeCallback(GLFWwindow* window, int w, int h);

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

public:
    void Run();
};
